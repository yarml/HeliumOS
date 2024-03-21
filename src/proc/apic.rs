use core::arch::x86_64::__cpuid;

pub fn id() -> u16 {
  let cpuid = unsafe { __cpuid(1) };
  (cpuid.ebx >> 24) as u16
}

pub mod cfgtb {
  use core::mem;

  use crate::acpi::tables::{AcpiHeader, Madt};
  pub fn acpi_handler(table: &AcpiHeader) {
    let madt = Madt::from(table);
    let parsed_len = mem::size_of::<Madt>();
    
  }
}
