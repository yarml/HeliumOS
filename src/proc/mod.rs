pub mod apic;
pub mod task;

use crate::{bootboot::bootboot, mem::virt::KVMSPACE};
use alloc::collections::BTreeMap;
use core::sync::atomic::AtomicUsize;
use spin::{Once, RwLock};
use x86_64::VirtAddr;

static STACK_TABLE_VPTR: VirtAddr =
  VirtAddr::new_truncate(KVMSPACE.as_u64() + 4 * 1024 * 1024 * 1024 * 1024);

static PROC_TABLE: Once<RwLock<BTreeMap<usize, ProcInfo>>> = Once::new();

pub fn is_primary() -> bool {
  apic::id() == bootboot().bspid as usize
}

struct ProcInfo {
  id: usize,

  tick: AtomicUsize,
}

impl ProcInfo {
  // Unsafe, the caller should make sure the stack table has been allocated
  pub unsafe fn stack<'a>() -> &'a mut VirtAddr {
    let ptr = STACK_TABLE_VPTR.as_mut_ptr::<VirtAddr>().add(apic::id());
    ptr.as_mut().unwrap()
  }
}

pub mod init {
  use super::{
    apic::{self, numcores},
    ProcInfo, PROC_TABLE, STACK_TABLE_VPTR,
  };
  use crate::println;
  use crate::{
    bootboot::kernel_stack_size,
    interrupts::ERROR_STACK_SIZE,
    mem::{self, gdt::KernelGlobalDescriptorTable, heap},
    sys::{self, pause},
  };
  use alloc::collections::BTreeMap;
  use core::{alloc::Layout, sync::atomic::AtomicUsize};
  use spin::{rwlock::RwLock, Once};
  use x86_64::{
    align_up,
    structures::paging::{Page, PageTableFlags},
    VirtAddr,
  };

  static IGNITION: Once<()> = Once::new();

  pub fn ignite() -> ! {
    // A few things to do before waking up the other procs
    // Like allocating space for the stack table
    let stack_table_pgn = align_up(
      (numcores() * core::mem::size_of::<VirtAddr>()) as u64,
      mem::PAGE_SIZE as u64,
    ) / mem::PAGE_SIZE as u64;
    mem::valloc(
      Page::from_start_address(STACK_TABLE_VPTR).unwrap(),
      stack_table_pgn as usize,
      PageTableFlags::WRITABLE,
    );

    // Allocating space for the GDT table
    KernelGlobalDescriptorTable::init();

    // TODO: Setting up the list of chores

    // Initializing the proc table
    PROC_TABLE.call_once(|| RwLock::new(BTreeMap::new()));
    // That's it for now

    IGNITION.call_once(|| ());
    wakeup()
  }

  // Wait for initialization
  pub fn wait() {
    while let None = IGNITION.get() {
      pause();
    }
    wakeup();
  }

  fn wakeup() -> ! {
    // This function is executed by all processors
    // after all essentials have been setup

    let id = apic::id();

    let stack = stack();
    *unsafe { ProcInfo::stack() } = stack;

    // Allocate the NMI, and DF stacks
    let nmistack = VirtAddr::new(
      heap::alloc(Layout::from_size_align(ERROR_STACK_SIZE, 16).unwrap())
        .as_ptr() as u64
        + ERROR_STACK_SIZE as u64,
    );
    let dfstack = VirtAddr::new(
      heap::alloc(Layout::from_size_align(ERROR_STACK_SIZE, 16).unwrap())
        .as_ptr() as u64
        + ERROR_STACK_SIZE as u64,
    );

    unsafe {
      KernelGlobalDescriptorTable::register(stack, nmistack, dfstack);
    }

    let pinfo = ProcInfo {
      id,
      tick: AtomicUsize::new(0),
    };
    {
      let mut proc_table = PROC_TABLE.get().unwrap().write();
      proc_table.insert(id, pinfo);
    }

    println!("Done");
    sys::event_loop()
  }

  fn stack() -> VirtAddr {
    let x: usize = 0;
    let x_ptr = &x as *const usize as u64;
    let stack_size = kernel_stack_size() as u64;
    let stack_base = if x_ptr >= 0u64.wrapping_sub(stack_size) {
      0
    } else {
      align_up(x_ptr, stack_size)
    };
    VirtAddr::new(stack_base)
  }
}
