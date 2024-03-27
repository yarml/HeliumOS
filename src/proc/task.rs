use super::{apic, ProcInfo};
use crate::elf::ElfFile;
use crate::println;
use crate::{
  elf::{self},
  fs::initrd,
  mem::{
    self, palloc, pfree, pfree_all,
    virt::{mapper::KernelMapError, KVMSPACE},
    vmap_many,
  },
};
use alloc::sync::Arc;
use alloc::{slice, vec::Vec};
use core::sync::atomic::{AtomicUsize, Ordering};
use spin::{rwlock::RwLock, Once, RwLockWriteGuard};
use x86_64::{
  align_up,
  registers::rflags::RFlags,
  structures::paging::{Page, PageTableFlags, PhysFrame, Size4KiB},
  VirtAddr,
};

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

const MAX_TICK_MISS: usize = 10;

static mut CLOCK: AtomicUsize = AtomicUsize::new(0);

static TASKS: Once<RwLock<TaskList>> = Once::new();

pub type TaskRef = Arc<RwLock<Task>>;
pub type TaskList = Vec<TaskRef>;
pub type TasksLock<'a, 'b> = &'a mut RwLockWriteGuard<'b, TaskList>;

pub(super) fn init() {
  TASKS.call_once(|| RwLock::new(Vec::new()));
}

fn tmp_local_reserve(
  offset: usize,
  size: usize,
  frames: &[PhysFrame<Size4KiB>],
) -> &mut [u8] {
  let start_page = Page::from_start_address(
    TMP_TASKLDMAP + (apic::id() * TMP_TASK_SIZE_PER_PROC) as u64,
  )
  .unwrap();

  // Maps here are writable, that's why they're mapped to begin with
  vmap_many(start_page, frames, PageTableFlags::WRITABLE);

  let ptr = (start_page.start_address().as_u64() + offset as u64) as *mut u8;

  unsafe { slice::from_raw_parts_mut(ptr, size) }
}

pub fn tick() {
  let pinfo = ProcInfo::instance();
  let mut tasks_lock = if pinfo.tick_miss > MAX_TICK_MISS {
    TASKS.get().unwrap().write()
  } else {
    match TASKS.get().unwrap().try_write() {
      Some(lock) => lock,
      None => {
        pinfo.tick_miss += 1;
        return;
      }
    }
  };

  pinfo.tick_miss = 0;

  // If we got the lock, then this is a clock tick
  let clock = unsafe { CLOCK.fetch_add(1, Ordering::Relaxed) };

  // If no task exists, run init
  if tasks_lock.len() == 0 {
    match Task::exec_initrd("/bin/init", &mut tasks_lock) {
      ExecResult::Success(id) => {
        println!("[Proc {}] Start init with ID: {}", apic::id(), id)
      }
      error => panic!("Could not run /bin/init: {:?}", error),
    }
  }
  let tasks_lock = tasks_lock.downgrade();

  if let Some(task_lock) = pinfo.current_task.take_if(|task_lock| {
    let task = task_lock.write();
    let TaskState::Running { since } = task.state else {
      unreachable!()
    };
    clock - since >= task.life_expectancy()
  }) {
    let mut task = task_lock.write();
    println!("[Proc {}] Dropping {}", apic::id(), task.id);
    task.state = TaskState::Pending;
  }

  // If there is still a task running, then that's it we're done here
  if let Some(task) = &pinfo.current_task {
    let t = task.read();
    println!("[Proc {}] Continuing {}", apic::id(), t.id);
    return;
  }

  // No task is running, let's look for one to run
  // TODO: Shamelessly using CSC1401 way of doing it instead of a binary heap
  // Figure out how to implement Ord for RwLock<Task>, then change this
  // to a binary heap

  let mut best_task: Option<(usize, RwLockWriteGuard<'_, Task>)> = None;
  for (i, task_lock) in tasks_lock.iter().enumerate() {
    let task = match task_lock.try_write() {
      Some(task) => task,
      None => continue,
    };
    if task.state != TaskState::Pending {
      continue;
    }
    if let Some((_, btask)) = &mut best_task {
      if task.priority > btask.priority {
        *btask = task;
      }
    } else {
      best_task = Some((i, task));
    }
  }

  if let Some((index, mut task)) = best_task {
    // We have a cadidate
    task.state = TaskState::Running { since: clock };
    pinfo.current_task = Some(tasks_lock[index].clone());
    println!("[Proc {}] Running {}", apic::id(), task.id);
  }
}

pub struct Task {
  id: usize,
  priority: usize,
  procstate: TaskProcState,
  memory_map: MemoryMap,
  state: TaskState,
}

impl Task {
  fn new_locked(
    procstate: TaskProcState,
    memory_map: MemoryMap,
    tasks_lock: TasksLock,
  ) -> usize {
    static mut IDCOUNT: RwLock<usize> = RwLock::new(0);

    let (task, id) = {
      let mut idcount_lock = unsafe { IDCOUNT.write() };
      *idcount_lock += 1;
      (
        Self {
          id: *idcount_lock,
          priority: 0,
          procstate,
          memory_map,
          state: TaskState::Pending,
        },
        *idcount_lock,
      )
    };
    tasks_lock.push(Arc::new(RwLock::new(task)));
    id
  }
  fn exec_initrd(path: &str, tasks_lock: TasksLock) -> ExecResult {
    if let Some(file) = initrd::open_file(path) {
      Self::exec_raw(file, tasks_lock)
    } else {
      ExecResult::FileError
    }
  }
  fn exec_raw(content: &[u8], tasks_lock: TasksLock) -> ExecResult {
    if let Some(elf) = unsafe { ElfFile::parse(content) } {
      Self::exec(elf, tasks_lock)
    } else {
      ExecResult::ParseError
    }
  }
  fn exec(elf: ElfFile, tasks_lock: TasksLock) -> ExecResult {
    let mut memory_map = MemoryMap::new();
    for entry in elf.header.iter_progheader() {
      match entry.ptype {
        elf::ProgramType::Load => {
          println!("LOAD");
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
              None => {
                return ExecResult::BinaryEnd(
                  entry.offset as usize,
                  entry.file_size as usize,
                )
              }
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
            );

            data.fill(0);
            data.copy_from_slice(content);
          } else {
            return ExecResult::AllocationError;
          }
        }
        other => {
          return ExecResult::UnsupportedDirective(other as u32);
        }
      }
    }

    // Stack
    {
      let page_start = Page::from_start_address(USERSTACK_TOP).unwrap();
      let pgn = USERSTACK_SIZE / mem::PAGE_SIZE;
      if let Some(frames) =
        memory_map.allocate(page_start, pgn, PageTableFlags::WRITABLE)
      {
        tmp_local_reserve(0, USERSTACK_SIZE, &frames).fill(0);
      } else {
        return ExecResult::AllocationError;
      }
    }

    let state = TaskProcState {
      instruction: elf.header.entrypoint(),
      stack: USERSTACK_BOTTOM,
      rflags: RFlags::INTERRUPT_FLAG,
    };

    let id = Task::new_locked(state, memory_map, tasks_lock);
    ExecResult::Success(id)
  }

  fn life_expectancy(&self) -> usize {
    (self.priority + 10) * apic::numcores()
  }
}

#[derive(Debug)]
pub enum ExecResult {
  Success(usize),
  FileError,
  ParseError,
  InsecureLoad(usize, usize),
  AllocationError,
  LargeAlignment(usize),
  LargeSize(usize),
  BinaryEnd(usize, usize),
  UnsupportedDirective(u32),
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
    let mut allocated_frames = Vec::new();

    for _ in 0..n {
      if let Some(frame) = palloc() {
        allocated_frames.push(frame);
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

pub struct TaskProcState {
  stack: u64,
  instruction: VirtAddr,
  rflags: RFlags,
}

#[derive(PartialEq)]
pub enum TaskState {
  Running { since: usize },
  Pending,
  Blocking,
}
