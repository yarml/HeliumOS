use crate::println;
use core::{
  arch::x86_64::__cpuid,
  ptr::{self, addr_of, addr_of_mut},
};

use alloc::{collections::BTreeMap, vec::Vec};
use spin::{Once, RwLock};
use x86_64::{
  structures::paging::{Page, PageTableFlags, PhysFrame, Size4KiB},
  PhysAddr, VirtAddr,
};

use crate::{
  acpi::tables::MadtEntryIoApic,
  mem::{self, virt::KVMSPACE},
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
static PROCNUM: RwLock<usize> = RwLock::new(0);

pub fn id() -> u16 {
  let cpuid = unsafe { __cpuid(1) };
  (cpuid.ebx >> 24) as u16
}
pub fn numproc() {
  let n = PROCNUM.read();
  println!("Number of proc: {}", n);
}

#[derive(PartialEq, PartialOrd, Eq, Ord, Debug)]
pub enum IoApicRedirectionSource {
  Irq(usize),
  Bus(usize),
}

struct IoApicInfo {
  regmap: &'static mut IoApicRegisterMap,
  id: usize,
  irq_base: usize,
  len: usize,
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
      regmap,
      id: entry.ioapicid as usize,
      irq_base: entry.gsib as usize,
      len,
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

  pub fn id(&mut self) -> usize {
    ((self.read(0) >> 24) & 0xF) as usize
  }
  pub fn maxredent(&mut self) -> usize {
    ((self.read(1) >> 16) & 0xFF) as usize
  }
  pub fn version(&mut self) -> usize {
    (self.read(1) & 0xFF) as usize
  }
}

pub mod cfgtb {
  use alloc::{collections::BTreeMap, vec::Vec};
  use spin::rwlock::RwLock;

  use crate::{
    acpi::tables::{
      AcpiHeader, Madt, MadtEntry, MadtEntryIoApic,
      MadtEntryIoApicInterruptSourceOverride,
    },
    println,
    proc::apic::PROCNUM,
  };

  use super::{IoApicInfo, IOAPIC, IOAPIC_REDIRECTION, VBASE};

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
          if !lapic.enabled() {
            continue;
          }
          let mut procnum_lock = PROCNUM.write();
          *procnum_lock += 1;
          // TODO: Procinfo here
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
        MadtEntry::Other(header) => {
          println!("\tUnsupported MADT entry: {:?}", header.mtype)
        }
      }
    }
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
