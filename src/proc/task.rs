use crate::{
  elf::{self, ProgramType},
  fs::initrd,
  mem::{
    self, palloc, pfree, pfree_all,
    virt::{mapper::KernelMapError, KVMSPACE},
    vmap_many,
  },
};
use alloc::{boxed::Box, slice, vec::Vec};
use spin::{rwlock::RwLock, Once, RwLockWriteGuard};
use x86_64::{
  align_up,
  structures::paging::{Page, PageTableFlags, PhysFrame, Size4KiB},
  VirtAddr,
};

use super::apic;

pub const USERSPACE_TOP: VirtAddr = VirtAddr::new_truncate(0x7fff_ffffffff);
pub const USERSTACK_SIZE: usize = 2 * 1024 * 1024;
pub const USERSTACK_BOTTOM: u64 = USERSPACE_TOP.as_u64() + 1;
pub const USERSTACK_TOP: VirtAddr =
  VirtAddr::new_truncate(USERSTACK_BOTTOM - USERSTACK_SIZE as u64);

// When loading a task, we need to set memory in user space to certain values,
// But we don't want to change the current mapping of the user space, so instead
// those frames will be temporarily mapped in this area to be filled with their initial
// data. Each core in the system has a defined region in this space and should not
// be able to use more than it.
const TMP_TASKLDMAP: VirtAddr =
  VirtAddr::new_truncate(KVMSPACE.as_u64() + 9 * 1024 * 1024 * 1024 * 1024);
const TMP_TASK_SIZE_PER_PROC: usize = 32 * 1024 * 1024 * 1024;

static TASKS: Once<RwLock<Vec<Box<Task>>>> = Once::new();

type TasksLock<'a, 'b> = &'a mut RwLockWriteGuard<'b, Vec<Box<Task>>>;

pub(super) fn init() {
  TASKS.call_once(|| RwLock::new(Vec::new()));
}

fn tmp_local_reserve(
  offset: usize,
  size: usize,
  frames: &[PhysFrame<Size4KiB>],
  flags: PageTableFlags,
) -> &mut [u8] {
  let start_page = Page::from_start_address(
    TMP_TASKLDMAP + (apic::id() * TMP_TASK_SIZE_PER_PROC) as u64,
  )
  .unwrap();

  vmap_many(start_page, frames, flags);

  let ptr = (start_page.start_address().as_u64() + offset as u64) as *mut u8;

  unsafe { slice::from_raw_parts_mut(ptr, size) }
}

pub fn tick() {
  let mut tasks_lock = TASKS.get().unwrap().write();

  // If no task is running, run init
  if tasks_lock.len() == 0 {
    match Task::exec_initrd("/bin/init", &mut tasks_lock) {
      ExecResult::Success => {}
      error => panic!("Could not run /bin/init: {:?}", error),
    }
  }
}

pub struct Task {
  id: usize,
  priority: usize,
  state: TaskState,
  memory_map: MemoryMap,
}

impl Task {
  fn new_locked(
    state: TaskState,
    memory_map: MemoryMap,
    tasks_lock: TasksLock,
  ) {
    static mut IDCOUNT: RwLock<usize> = RwLock::new(0);

    let task = {
      let mut idcount_lock = unsafe { IDCOUNT.write() };
      *idcount_lock += 1;
      Self {
        id: *idcount_lock,
        priority: 0,
        state,
        memory_map,
      }
    };
    tasks_lock.push(Box::new(task));
  }
  fn exec_initrd(path: &str, tasks_lock: TasksLock) -> ExecResult {
    if let Some(file) = initrd::open_file(path) {
      Self::exec_raw(file, tasks_lock)
    } else {
      ExecResult::FileError
    }
  }
  fn exec_raw(content: &[u8], tasks_lock: TasksLock) -> ExecResult {
    if let Some(elf) = unsafe { elf::FileHeader::parse(content) } {
      Self::exec(elf, tasks_lock)
    } else {
      ExecResult::ParseError
    }
  }
  fn exec(elf: &elf::FileHeader, tasks_lock: TasksLock) -> ExecResult {
    let mut memory_map = MemoryMap::new();
    for entry in elf.iter_progheader() {
      match entry.ptype {
        elf::ProgramType::Load => {
          if entry.align as usize > mem::PAGE_SIZE {
            return ExecResult::LargeAlignment(entry.align as usize);
          }
          if entry.vadr + entry.mem_size >= USERSPACE_TOP.as_u64() {
            return ExecResult::InsecureLoad(
              entry.vadr as usize,
              entry.mem_size as usize,
            );
          }
          if entry.mem_size as usize > TMP_TASK_SIZE_PER_PROC {
            return ExecResult::LargeSize(entry.mem_size as usize);
          }
          let content =
            match elf.slice(entry.offset as usize, entry.file_size as usize) {
              None => return ExecResult::LargeSize(entry.file_size as usize),
              Some(content) => content,
            };

          let flags = entry.memory_flags() | PageTableFlags::USER_ACCESSIBLE;

          let page_start =
            Page::containing_address(VirtAddr::new_truncate(entry.vadr));
          let pgn = align_up(
            entry.mem_size + entry.vadr - page_start.start_address().as_u64(),
            mem::PAGE_SIZE as u64,
          ) as usize
            / mem::PAGE_SIZE;
          if let Some(frames) = memory_map.allocate(page_start, pgn, flags) {
            let data = tmp_local_reserve(
              (page_start.start_address().as_u64() - entry.vadr) as usize,
              entry.mem_size as usize,
              &frames,
              flags,
            );

            data.fill(0);
            data.copy_from_slice(content);
          } else {
            return ExecResult::AllocationError;
          }
        }
        other => {
          return ExecResult::UnsupportedDirective(other);
        }
      }
    }

    // Stack
    memory_map.allocate(
      Page::from_start_address(USERSTACK_TOP).unwrap(),
      USERSTACK_SIZE / mem::PAGE_SIZE,
      PageTableFlags::WRITABLE,
    );

    let state = TaskState {
      instruction: elf.entrypoint(),
      stack: USERSTACK_BOTTOM,
      rflags: 0x200,
    };

    Task::new_locked(state, memory_map, tasks_lock);
    ExecResult::Success
  }
}

#[derive(Debug)]
pub enum ExecResult {
  Success,
  FileError,
  ParseError,
  InsecureLoad(usize, usize),
  AllocationError,
  LargeAlignment(usize),
  LargeSize(usize),
  UnsupportedDirective(ProgramType),
}

pub struct MemoryMap {
  mappings: Vec<MemoryMapping>,
}

impl MemoryMap {
  pub fn new() -> Self {
    Self {
      mappings: Vec::new(),
    }
  }

  pub fn allocate(
    &mut self,
    vadr: Page<Size4KiB>,
    n: usize,
    flags: PageTableFlags,
  ) -> Option<Vec<PhysFrame<Size4KiB>>> {
    let allocated_frames = Vec::new();

    for _ in 0..n {
      if let Some(frame) = palloc() {
        if let None = self.add_raw(vadr, frame, 1, flags, true) {
          pfree_all(&allocated_frames);
          return None;
        }
      } else {
        pfree_all(&allocated_frames);
        return None;
      }
    }

    Some(allocated_frames)
  }

  pub fn add(
    &mut self,
    vadr: Page<Size4KiB>,
    padr: PhysFrame<Size4KiB>,
    n: usize,
    flags: PageTableFlags,
  ) -> Option<&MemoryMapping> {
    self.add_raw(vadr, padr, n, flags, false)
  }

  fn add_raw(
    &mut self,
    vadr: Page<Size4KiB>,
    padr: PhysFrame<Size4KiB>,
    n: usize,
    flags: PageTableFlags,
    allocated: bool,
  ) -> Option<&MemoryMapping> {
    self.mappings.push(MemoryMapping {
      vadr,
      padr,
      n,
      flags,
      allocated,
    });

    self.mappings.last()
  }
}

pub struct MemoryMapping {
  vadr: Page<Size4KiB>,
  padr: PhysFrame<Size4KiB>,
  n: usize,
  flags: PageTableFlags,
  allocated: bool,
}
impl MemoryMapping {
  pub fn apply(&self) -> Result<(), KernelMapError> {
    mem::vmap(self.vadr, self.padr, self.n * mem::PAGE_SIZE, self.flags)
  }
}

impl Drop for MemoryMapping {
  fn drop(&mut self) {
    if self.allocated {
      pfree(self.padr);
    }
  }
}

pub struct TaskState {
  stack: u64,
  instruction: VirtAddr,
  rflags: u64,
}
