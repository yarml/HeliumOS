use super::VMemMap;
use crate::{
  acpi::getvadr, proc::apic::IoApicRedirectionSource, utils::unchecked_cast,
};
use core::{mem, ptr, slice, str::from_utf8};
use x86_64::PhysAddr;

#[repr(C, packed)]
pub struct AcpiHeader {
  signature: [u8; 4],
  pub len: u32,
  rev: u8,
  checksum: u8,
  oem: [u8; 6],
  oem_table_id: [u8; 8],
  oem_rev: u32,
  creator_id: u32,
  creator_rev: u32,
}
impl AcpiHeader {
  pub fn verify_checksum(&self) -> bool {
    let slice = unsafe {
      slice::from_raw_parts(self as *const Self as *const u8, self.len as usize)
    };
    slice.iter().fold(0u8, |acc, c| acc.wrapping_add(*c)) == 0
  }
  pub fn signature(&self) -> &str {
    from_utf8(&self.signature).expect("ACPI table signature not UTF-8")
  }
}

#[repr(C, packed)]
pub struct Xsdt {
  header: AcpiHeader,
}

impl Xsdt {
  pub fn from(header: &AcpiHeader) -> &Xsdt {
    debug_assert!(header.verify_checksum());
    debug_assert_eq!(header.signature(), "XSDT");
    unsafe { unchecked_cast(header) }
  }

  pub fn len(&self) -> usize {
    (self.header.len as usize - mem::size_of::<AcpiHeader>()) / 8
  }

  pub fn sslist(&self, index: usize) -> PhysAddr {
    assert!(index < self.len());
    let ptr =
      unsafe { ((self as *const Self).add(1) as *const u64).add(index) };
    PhysAddr::new(unsafe { ptr::read_unaligned(ptr) })
  }

  pub fn at(
    &self,
    vmemmap: &mut VMemMap,
    pgindex: &mut usize,
    index: usize,
  ) -> &AcpiHeader {
    getvadr(vmemmap, self.sslist(index), pgindex)
  }
}

#[repr(C, packed)]
pub struct Madt {
  header: AcpiHeader,
  lapic_adr: u32,
  flags: u32,
}

impl Madt {
  pub fn from(header: &AcpiHeader) -> &Madt {
    debug_assert!(header.verify_checksum());
    debug_assert_eq!(header.signature(), "APIC");
    unsafe { unchecked_cast(header) }
  }

  pub fn iter(&self) -> MadtIterator {
    MadtIterator {
      madt: self,
      cursor: mem::size_of::<Madt>(),
    }
  }
}

#[repr(u8)]
#[allow(dead_code)]
#[derive(Debug)]
pub enum MadtType {
  LocalApic = 0,
  IoApic = 1,
  IoApicInterruptSourceOverride = 2,
  IoApicNmiSource = 3,
  LocalApicNmi = 4,
  LocalApicAddressOverride = 5,
  LocalApicExt2 = 9,
}

#[repr(C, packed)]
pub struct MadtEntryHeader {
  pub mtype: MadtType,
  len: u8,
}

#[repr(C, packed)]
pub struct MadtEntryLocalApic {
  header: MadtEntryHeader,
  pub procid: u8,
  pub apicid: u8,
  flags: u32,
}
impl MadtEntryLocalApic {
  pub fn enabled(&self) -> bool {
    self.flags != 0
  }
}

#[repr(C, packed)]
pub struct MadtEntryIoApic {
  header: MadtEntryHeader,
  pub ioapicid: u8,
  res: u8,
  pub ioapic_adr: u32,
  pub gsib: u32,
}

#[repr(C, packed)]
pub struct MadtEntryIoApicInterruptSourceOverride {
  header: MadtEntryHeader,
  bus_src: u8,
  irq_src: u8,
  gsi: u32,
  flags: u16,
}
impl MadtEntryIoApicInterruptSourceOverride {
  pub fn source(&self) -> IoApicRedirectionSource {
    if self.bus_src != 0 {
      IoApicRedirectionSource::Bus(self.bus_src as usize)
    } else {
      IoApicRedirectionSource::Irq(self.irq_src as usize)
    }
  }
  pub fn destination(&self) -> usize {
    self.gsi as usize
  }
}

#[repr(C, packed)]
pub struct MadtEntryLocalApicNmi {
  pub id: u8,
  pub flags: u16,
  pub lint: u8,
}

pub enum MadtEntry<'a> {
  LocalApic(&'a MadtEntryLocalApic),
  IoApic(&'a MadtEntryIoApic),
  IoApicInterruptSourceOverride(&'a MadtEntryIoApicInterruptSourceOverride),
  LocalApicNmi(&'a MadtEntryLocalApicNmi),
  Other(&'a MadtEntryHeader),
}

pub struct MadtIterator<'a> {
  madt: &'a Madt,
  cursor: usize,
}

impl<'a> Iterator for MadtIterator<'a> {
  type Item = MadtEntry<'a>;

  fn next(&mut self) -> Option<Self::Item> {
    if self.cursor >= self.madt.header.len as usize {
      None
    } else {
      let entry_header = unsafe {
        ((self.madt as *const Madt as *const u8).add(self.cursor)
          as *const MadtEntryHeader)
          .as_ref()
      }
      .unwrap();
      self.cursor += entry_header.len as usize;

      unsafe {
        Some(match entry_header.mtype {
          MadtType::LocalApic => {
            MadtEntry::LocalApic(unchecked_cast(entry_header))
          }
          MadtType::IoApic => MadtEntry::IoApic(unchecked_cast(entry_header)),
          MadtType::IoApicInterruptSourceOverride => {
            MadtEntry::IoApicInterruptSourceOverride(unchecked_cast(
              entry_header,
            ))
          }
          MadtType::LocalApicNmi => {
            MadtEntry::LocalApicNmi(unchecked_cast(entry_header))
          }
          _ => MadtEntry::Other(entry_header),
        })
      }
    }
  }
}
