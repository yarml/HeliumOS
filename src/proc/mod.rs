pub mod apic;
pub mod task;

use crate::{
  bootboot::bootboot, interrupts::ERROR_STACK_SIZE, mem::virt::KVMSPACE,
};
use alloc::collections::BTreeMap;
use core::sync::atomic::AtomicUsize;
use spin::{Once, RwLock};
use x86_64::VirtAddr;

static STACK_TABLE_VPTR: VirtAddr =
  VirtAddr::new_truncate(KVMSPACE.as_u64() + 4 * 1024 * 1024 * 1024 * 1024);
static NMI_STACKS_VPTR: VirtAddr =
  VirtAddr::new_truncate(KVMSPACE.as_u64() + 7 * 1024 * 1024 * 1024 * 1024);
static DF_STACKS_VPTR: VirtAddr =
  VirtAddr::new_truncate(KVMSPACE.as_u64() + 7 * 1024 * 1024 * 1024 * 1024);
static PROC_TABLE: Once<RwLock<BTreeMap<usize, ProcInfo>>> = Once::new();

pub fn is_primary() -> bool {
  apic::id() == bootboot().bspid as usize
}

struct ProcInfo {
  id: usize,

  tick: AtomicUsize,
}

impl ProcInfo {
  // unsafe: The caller should make sure the stack table has been allocated
  pub unsafe fn stack<'a>() -> &'a mut VirtAddr {
    let ptr = STACK_TABLE_VPTR.as_mut_ptr::<VirtAddr>().add(apic::id());
    ptr.as_mut().unwrap()
  }

  pub fn nmi_stack() -> VirtAddr {
    NMI_STACKS_VPTR + (ERROR_STACK_SIZE * apic::id()) as u64
  }
  pub fn df_stack() -> VirtAddr {
    DF_STACKS_VPTR + (ERROR_STACK_SIZE * apic::id()) as u64
  }
}

pub mod init {
  use super::{
    apic::{self, numcores},
    ProcInfo, DF_STACKS_VPTR, NMI_STACKS_VPTR, PROC_TABLE, STACK_TABLE_VPTR,
  };
  use crate::{
    bootboot::kernel_stack_size,
    interrupts::ERROR_STACK_SIZE,
    mem::gdt::KernelGlobalDescriptorTable,
    sys::{self, pause},
  };
  use crate::{mem::valloc_ktable, println};
  use alloc::collections::BTreeMap;
  use core::sync::atomic::AtomicUsize;
  use spin::{rwlock::RwLock, Once};
  use x86_64::{align_up, VirtAddr};

  static IGNITION: Once<()> = Once::new();

  pub fn ignite() -> ! {
    let numcores = numcores();
    // A few things to do before waking up the other procs
    // Like allocating space for the stack table
    valloc_ktable::<VirtAddr>(STACK_TABLE_VPTR, numcores);

    // Allocating space for the GDT table
    KernelGlobalDescriptorTable::init();

    // Allocating NMI & DF stacks
    valloc_ktable::<[u8; ERROR_STACK_SIZE]>(NMI_STACKS_VPTR, numcores);
    valloc_ktable::<[u8; ERROR_STACK_SIZE]>(DF_STACKS_VPTR, numcores);

    // TODO: Setting up the list of chores

    // Initializing the proc table
    PROC_TABLE.call_once(|| RwLock::new(BTreeMap::new()));
    // That's it for now

    IGNITION.call_once(|| ());
    wakeup()
  }

  // Wait for initialization
  pub fn wait() -> ! {
    while let None = IGNITION.get() {
      pause();
    }
    wakeup()
  }

  fn wakeup() -> ! {
    // This function is executed by all processors
    // after all essentials have been setup

    let id = apic::id();

    let stack = stack();
    *unsafe { ProcInfo::stack() } = stack;

    // Allocate the NMI, and DF stacks
    let nmistack = ProcInfo::nmi_stack();
    let dfstack = ProcInfo::df_stack();

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

    // waitall();

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

  // Wait until all other processors were inserted into the PROC_TABLE
  fn waitall() {
    let numcores = numcores();
    loop {
      let proc_table = PROC_TABLE.get().unwrap().read();
      let initnum = proc_table.keys().count();
      if numcores == initnum {
        return;
      }
      drop(proc_table);
      pause();
    }
  }
}
