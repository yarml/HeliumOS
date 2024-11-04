pub mod apic;
mod syscall;
pub mod task;

use crate::{
  bootboot::bootboot, interrupts::ERROR_STACK_SIZE, mem::virt::KVMSPACE,
};
use alloc::collections::BTreeMap;
use spin::{Once, RwLock};
use x86_64::VirtAddr;

use self::task::TaskRef;

static STACK_TABLE_VPTR: VirtAddr =
  VirtAddr::new_truncate(KVMSPACE.as_u64() + 4 * 1024 * 1024 * 1024 * 1024);
static NMI_STACKS_VPTR: VirtAddr =
  VirtAddr::new_truncate(KVMSPACE.as_u64() + 7 * 1024 * 1024 * 1024 * 1024);
static DF_STACKS_VPTR: VirtAddr =
  VirtAddr::new_truncate(KVMSPACE.as_u64() + 8 * 1024 * 1024 * 1024 * 1024);
static PROC_TABLE: Once<RwLock<BTreeMap<usize, ProcInfo>>> = Once::new();

pub fn is_primary() -> bool {
  apic::id() == bootboot().bspid as usize
}

struct ProcInfo {
  _id: usize,
  tick_miss: usize,
  current_task: Option<TaskRef>,
  basic: BasicProcInfo,
  schedule_quantum: usize,
}

// Exposed to assembly via KGSBASE
// Values in here are just scratch for assembly code, they should not be relied upon
#[repr(C)]
struct BasicProcInfo {
  save_user_rsp: u64,
  kernel_rsp: u64,
}

impl ProcInfo {
  // unsafe: The caller should make sure the stack table has been allocated
  pub unsafe fn stack<'a>() -> &'a mut VirtAddr {
    let ptr = STACK_TABLE_VPTR.as_mut_ptr::<VirtAddr>().add(apic::id());
    ptr.as_mut().unwrap()
  }

  pub fn nmi_stack() -> VirtAddr {
    NMI_STACKS_VPTR + (ERROR_STACK_SIZE * (apic::id() + 1)) as u64
  }
  pub fn df_stack() -> VirtAddr {
    DF_STACKS_VPTR + (ERROR_STACK_SIZE * (apic::id() + 1)) as u64
  }

  // This should be safe
  pub fn instance() -> &'static mut ProcInfo {
    let mut proc_table = PROC_TABLE.get().unwrap().write();
    unsafe {
      (proc_table.get_mut(&apic::id()).unwrap() as *mut ProcInfo).as_mut()
    }
    .unwrap()
  }
}

pub mod init {
  use super::{
    apic::{self, numcores},
    ProcInfo, DF_STACKS_VPTR, NMI_STACKS_VPTR, PROC_TABLE, STACK_TABLE_VPTR,
  };
  use crate::{
    bootboot::kernel_stack_size,
    dev::framebuffer::debug_set_pixel,
    interrupts::{self, ERROR_STACK_SIZE},
    late_start,
    mem::gdt::KernelGlobalDescriptorTable,
    proc::{is_primary, syscall, task, BasicProcInfo},
    sys::{self, pause},
  };
  use crate::{mem::valloc_ktable, println};
  use alloc::collections::BTreeMap;
  use spin::{rwlock::RwLock, Once};
  use x86_64::{align_up, registers::model_specific::KernelGsBase, VirtAddr};

  static IGNITION: Once<()> = Once::new();
  static LATE_IGNITION: Once<()> = Once::new();

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

    // Setting up tasks
    task::init();

    // Initializing the proc table
    PROC_TABLE.call_once(|| RwLock::new(BTreeMap::new()));

    // That's it for now
    IGNITION.call_once(|| ());
    let id = apic::id();
    debug_set_pixel(1, id + 1, (255, 255, 128).into());
    debug_set_pixel(2, id + 1, (255, 128, 255).into());
    debug_set_pixel(3, id + 1, (128, 255, 255).into());
    wakeup()
  }

  // Wait for initialization
  pub fn wait() -> ! {
    while IGNITION.get().is_none() {
      pause();
    }
    loop {
      pause();
    }
    //wakeup()
  }

  fn latewait() {
    while LATE_IGNITION.get().is_none() {
      pause();
    }
  }

  fn wakeup() -> ! {
    // This function is executed by all processors
    // after all essentials have been setup

    let id = apic::id();
    debug_set_pixel(10, 0, (255, 255, 255).into());
    debug_set_pixel(10, id + 1, (255, 0, 255).into());

    let stack = stack();
    *unsafe { ProcInfo::stack() } = stack;
    debug_set_pixel(11, id + 1, (255, 255, 0).into());

    // Allocate the NMI, and DF stacks
    let nmistack = ProcInfo::nmi_stack();
    debug_set_pixel(12, id + 1, (255, 0, 255).into());
    let dfstack = ProcInfo::df_stack();
    debug_set_pixel(13, id + 1, (255, 255, 0).into());

    unsafe {
      KernelGlobalDescriptorTable::register(stack, nmistack, dfstack);
    }
    debug_set_pixel(14, id + 1, (255, 0, 255).into());

    let pinfo = ProcInfo {
      _id: id,
      tick_miss: 0,
      current_task: None,
      schedule_quantum: 0,
      basic: BasicProcInfo {
        save_user_rsp: 0,
        kernel_rsp: stack.as_u64(),
      },
    };
    {
      debug_set_pixel(15, id + 1, (255, 0, 0).into());
      let mut proc_table = PROC_TABLE.get().unwrap().write();
      proc_table.insert(id, pinfo);
    }
    debug_set_pixel(16, id + 1, (0, 0, 255).into());
    let pinfo_adr = &ProcInfo::instance().basic as *const BasicProcInfo as u64;
    // Set KERNEL_GS_BASE
    KernelGsBase::write(VirtAddr::new(pinfo_adr));
    debug_set_pixel(17, id + 1, (255, 0, 255).into());

    ProcInfo::instance().schedule_quantum = apic::init();
    debug_set_pixel(18, id + 1, (255, 255, 0).into());
    interrupts::load();
    debug_set_pixel(19, id + 1, (255, 0, 255).into());
    syscall::enable();
    debug_set_pixel(20, id + 1, (255, 255, 0).into());

    if is_primary() {
      late_start();
      LATE_IGNITION.call_once(|| ());
    } else {
      latewait();
    }
    debug_set_pixel(5, 0, (255, 128, 128).into());
    debug_set_pixel(5, id + 1, (255, 128, 128).into());

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
