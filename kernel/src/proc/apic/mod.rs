pub mod regmap;

use crate::dev::framebuffer::{debug_printbin, debug_set_pixel};
use crate::interrupts::pic::{self, PITCalib};
use crate::interrupts::Vectors;
use crate::println;
use crate::proc::apic;
use crate::proc::apic::regmap::{LocalApicRegisterMap, TimerMode};
use crate::proc::task::QUANTUM_MS;
use crate::{
  acpi::tables::{MadtEntryIoApic, MadtEntryLocalApicNmi},
  mem::{self, virt::KVMSPACE},
  proc::is_primary,
  sys::pause,
};
use alloc::{collections::BTreeMap, vec::Vec};
use core::usize;
use core::{
  arch::x86_64::__cpuid,
  ptr::{self, addr_of, addr_of_mut},
};
use spin::{Once, RwLock};
use x86_64::{
  structures::paging::{
    mapper::MapperFlushAll, Page, PageTableFlags, PhysFrame, Size4KiB,
  },
  PhysAddr, VirtAddr,
};

const VBASE: Page<Size4KiB> = unsafe {
  Page::from_start_address_unchecked(VirtAddr::new(
    KVMSPACE.as_u64() + 1024 * 1024 * 1024 * 1024 + 513 * 1024 * 1024 * 1024,
  ))
};

static IOAPIC_REDIRECTION: Once<
  RwLock<BTreeMap<IoApicRedirectionSource, usize>>,
> = Once::new();
static IOAPIC: Once<RwLock<Vec<IoApicInfo>>> = Once::new();
static APIC_CONFIG: Once<RwLock<Vec<LocalApicNmiConfig>>> = Once::new();
static NUMCORES: RwLock<usize> = RwLock::new(0);

const APIC_VBASE: VirtAddr = VirtAddr::new_truncate(
  KVMSPACE.as_u64() + 1024 * 1024 * 1024 * 1024 + 512 * 1024 * 1024 * 1024,
);
const APIC_BASE: PhysAddr = PhysAddr::new_truncate(0xFEE00000);

pub fn id() -> usize {
  let cpuid = unsafe { __cpuid(1) };
  (cpuid.ebx >> 24) as usize
}

pub fn numcores() -> usize {
  let numcores = NUMCORES.read();
  *numcores
}

struct APICTimerCalib;

impl PITCalib for APICTimerCalib {
  fn measure() -> usize {
    LocalApicRegisterMap::get().timer_current()
  }
}

// Called once per core
pub(super) fn init() -> usize {
  let id = apic::id();
  static MAPPED: Once<()> = Once::new();

  if !is_primary() {
    while MAPPED.get().is_none() {
      pause()
    }
    MapperFlushAll::new().flush_all();
  } else {
    debug_set_pixel(10, 20 + id, (255, 255, 255).into());
    println!("Mapping APIC registers: {:x}", APIC_VBASE.as_u64());
    mem::vmap(
      Page::from_start_address(APIC_VBASE).unwrap(),
      PhysFrame::from_start_address(APIC_BASE).unwrap(),
      mem::PAGE_SIZE,
      PageTableFlags::empty(),
    )
    .expect("Could not map APIC registers into virtual memory");
    debug_set_pixel(11, 20 + id, (255, 0, 255).into());
    MAPPED.call_once(|| ());
  }

  let apic_msr = LocalApicRegisterMap::get();

  debug_set_pixel(12, 20 + id, (255, 255, 255).into());
  apic_msr.error_setup(Vectors::LocalApicError.into());
  debug_set_pixel(13, 20 + id, (255, 0, 255).into());
  apic_msr.spurious_setup(Vectors::LocalApicSpurious.into());
  debug_set_pixel(14, 20 + id, (255, 255, 255).into());
  apic_msr.timer_setup(Vectors::LocalApicTimer.into(), TimerMode::Periodic, 0);
  debug_set_pixel(15, 20 + id, (255, 0, 255).into());
  apic_msr.timer_reset(usize::MAX);
  println!("[Proc{}] Calibrating APIC", id);
  debug_set_pixel(16, 20 + id, (255, 255, 255).into());
  let pit_calib = pic::pit_calib_sleep::<APICTimerCalib>();
  debug_set_pixel(17, 20 + id, (255, 0, 255).into());
  let apic_10ms = pit_calib.0 - pit_calib.1;
  debug_printbin(50, id + 1, apic_10ms);
  println!("APIC 10ms = {} tick", apic_10ms);
  let quantum = apic_10ms * QUANTUM_MS / 10;
  apic_msr.timer_reset(quantum);
  debug_set_pixel(18, 20 + id, (255, 255, 255).into());

  // Setup LINT0 & LINT1
  if let Some(apic_config) = APIC_CONFIG.get() {
    let apic_config_lock = apic_config.read();
    for cfg in apic_config_lock.iter() {
      let (lint, flags) = match cfg {
        LocalApicNmiConfig::All { lint, flags } => (*lint, *flags),
        LocalApicNmiConfig::Specific { id, lint, flags } => {
          if *id == apic::id() {
            (*lint, *flags)
          } else {
            continue;
          }
        }
      };
      apic_msr.lint_setup(lint, flags);
    }
  }
  quantum
}

#[derive(Debug)]
pub enum LocalApicNmiConfig {
  All { lint: usize, flags: u16 },
  Specific { id: usize, lint: usize, flags: u16 },
}

impl From<&MadtEntryLocalApicNmi> for LocalApicNmiConfig {
  fn from(entry: &MadtEntryLocalApicNmi) -> Self {
    if entry.id == 0xFF {
      Self::All {
        lint: entry.lint as usize,
        flags: entry.flags,
      }
    } else {
      Self::Specific {
        id: entry.id as usize,
        lint: entry.lint as usize,
        flags: entry.flags,
      }
    }
  }
}

#[derive(PartialEq, PartialOrd, Eq, Ord, Debug)]
pub enum IoApicRedirectionSource {
  Irq(usize),
  Bus(usize),
}

struct IoApicInfo {
  _regmap: &'static mut IoApicRegisterMap,
  _id: usize,
  _irq_base: usize,
  _len: usize,
}

impl IoApicInfo {
  pub fn new(entry: &MadtEntryIoApic, vadr: Page<Size4KiB>) -> Self {
    mem::vmap(
      vadr,
      PhysFrame::from_start_address(PhysAddr::new(entry.ioapic_adr as u64))
        .unwrap(),
      mem::PAGE_SIZE,
      PageTableFlags::empty(),
    )
    .expect("Could not map memory for IoApic info");

    let regmap = unsafe {
      vadr
        .start_address()
        .as_mut_ptr::<IoApicRegisterMap>()
        .as_mut()
    }
    .unwrap();

    let len = regmap.maxredent() + 1;

    Self {
      _regmap: regmap,
      _id: entry.ioapicid as usize,
      _irq_base: entry.gsib as usize,
      _len: len,
    }
  }
}

#[repr(C, packed)]
struct IoApicRegisterMap {
  regsel: u32,
  res: [u32; 3],
  regwin: u32,
}

impl IoApicRegisterMap {
  pub fn read(&mut self, offset: u32) -> u32 {
    unsafe {
      // unaligned_volatile_store(&mut self.regsel as *mut u32, offset);
      // unaligned_volatile_load(&self.regwin as *const u32)
      // Rust is a bitch sometimes, the above does not work because reasons
      // The pointers are actually aligned, it just can't see it.
      let regsel_ptr = addr_of_mut!(self.regsel);
      let regwin_ptr = addr_of!(self.regwin);
      debug_assert!(regsel_ptr.is_aligned());
      debug_assert!(regwin_ptr.is_aligned());
      ptr::write_volatile(regsel_ptr, offset);
      ptr::read_volatile(regwin_ptr)
    }
  }

  // pub fn id(&mut self) -> usize {
  //   ((self.read(0) >> 24) & 0xF) as usize
  // }
  pub fn maxredent(&mut self) -> usize {
    ((self.read(1) >> 16) & 0xFF) as usize
  }
  // pub fn version(&mut self) -> usize {
  //   (self.read(1) & 0xFF) as usize
  // }
}

pub mod cfgtb {
  use alloc::{collections::BTreeMap, vec::Vec};
  use spin::rwlock::RwLock;

  use crate::{
    acpi::tables::{
      AcpiHeader, Madt, MadtEntry, MadtEntryIoApic,
      MadtEntryIoApicInterruptSourceOverride, MadtEntryLocalApicNmi,
    },
    println,
    proc::apic::NUMCORES,
  };

  use super::{
    IoApicInfo, LocalApicNmiConfig, APIC_CONFIG, IOAPIC, IOAPIC_REDIRECTION,
    VBASE,
  };

  pub fn acpi_handler(table: &AcpiHeader) {
    let madt = Madt::from(table);
    println!("MADT:");
    for entry in madt.iter() {
      match entry {
        MadtEntry::LocalApic(lapic) => {
          println!(
            "\tLocalApic: Proc ID: {}, Apic ID: {}, enabled: {}",
            lapic.procid,
            lapic.apicid,
            lapic.enabled()
          );
          let mut numcores = NUMCORES.write();
          *numcores += 1;
        }
        MadtEntry::IoApic(ioapic) => {
          println!("\tIoApic: ID: {}", ioapic.ioapicid);
          register_ioapic(ioapic);
        }
        MadtEntry::IoApicInterruptSourceOverride(iso) => {
          println!(
            "\tIoApic redirection: {:?} -> {}",
            iso.source(),
            iso.destination()
          );
          register_ioapic_redirection(iso);
        }
        MadtEntry::LocalApicNmi(config) => {
          println!(
            "\tLocalApicNmiConfig: {:?}",
            LocalApicNmiConfig::from(config)
          );
          register_apic_config(config);
        }
        MadtEntry::Other(header) => {
          println!("\tUnsupported MADT entry: {:?}", header.mtype)
        }
      }
    }
  }

  fn register_apic_config(config: &MadtEntryLocalApicNmi) {
    APIC_CONFIG.call_once(|| RwLock::new(Vec::new()));
    let mut apic_config_lock = APIC_CONFIG.get().unwrap().write();
    apic_config_lock.push(LocalApicNmiConfig::from(config));
  }

  fn register_ioapic(ioapic: &MadtEntryIoApic) {
    IOAPIC.call_once(|| RwLock::new(Vec::new()));
    let mut ioapic_lock = IOAPIC.get().unwrap().write();

    let vadr = VBASE + ioapic_lock.len() as u64;

    let info = IoApicInfo::new(ioapic, vadr);
    ioapic_lock.push(info);
  }

  fn register_ioapic_redirection(iso: &MadtEntryIoApicInterruptSourceOverride) {
    IOAPIC_REDIRECTION.call_once(|| RwLock::new(BTreeMap::new()));
    let mut ioapic_redirections = IOAPIC_REDIRECTION.get().unwrap().write();
    ioapic_redirections.insert(iso.source(), iso.destination());
  }
}
