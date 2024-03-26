pub mod tables;

use self::tables::{AcpiHeader, Xsdt};
use crate::{
  bootboot::bootboot,
  cfgtb,
  mem::{self, virt::KVMSPACE},
  println,
};
use alloc::collections::BTreeMap;
use x86_64::{
  align_up,
  structures::paging::{Page, PageTableFlags, PhysFrame, Size4KiB},
  PhysAddr, VirtAddr,
};

const TABLE_VBASE: Page<Size4KiB> = unsafe {
  Page::from_start_address_unchecked(VirtAddr::new(
    KVMSPACE.as_u64() + 1024 * 1024 * 1024 * 1024 + 768 * 1024 * 1024 * 1024,
  ))
};

pub fn init() {
  let bootboot = bootboot();
  let mut vmemmap = BTreeMap::new();
  let mut pgindex = 0;

  let xsdt = getvadr(
    &mut vmemmap,
    PhysAddr::new(unsafe { bootboot.arch.x86_64 }.acpi_ptr),
    &mut pgindex,
  );

  walk_recursive(xsdt, &mut vmemmap, &mut pgindex);
}

type VMemMap = BTreeMap<PhysFrame<Size4KiB>, Page<Size4KiB>>;

fn walk_recursive(
  head: &AcpiHeader,
  vmemmap: &mut VMemMap,
  pgindex: &mut usize,
) {
  if !head.verify_checksum() {
    println!("Invalid checksum");
    return;
  }

  match head.signature() {
    // XSDT never handled by config table
    b"XSDT" => {
      let xsdt = Xsdt::from(head);
      for i in 0..xsdt.len() {
        let next_table = xsdt.at(vmemmap, pgindex, i);
        walk_recursive(next_table, vmemmap, pgindex);
      }
    }
    other_sig => {
      if !cfgtb::acpi::call(head.signature(), head) {
        println!("No ACPI handler for {:?}", other_sig);
      }
    }
  }
}

fn getvadr(
  vmemmap: &mut VMemMap,
  padr: PhysAddr,
  pgindex: &mut usize,
) -> &'static AcpiHeader {
  let frame = PhysFrame::<Size4KiB>::containing_address(padr);
  let modulo = padr.as_u64() % mem::PAGE_SIZE as u64;
  if let Some(page) = vmemmap.get(&frame) {
    unsafe {
      (page.start_address() + modulo)
        .as_ptr::<AcpiHeader>()
        .as_ref()
    }
    .unwrap()
  } else {
    map_table(vmemmap, padr, pgindex)
  }
}

fn map_table(
  vmemmap: &mut VMemMap,
  padr: PhysAddr,
  pgindex: &mut usize,
) -> &'static AcpiHeader {
  const ERROR_MSG: &str = "Could not map ACPI table";

  let frame = PhysFrame::<Size4KiB>::containing_address(padr);
  let page = TABLE_VBASE + *pgindex as u64;
  mem::vmap(page, frame, mem::PAGE_SIZE, PageTableFlags::empty())
    .expect(ERROR_MSG);

  let table = unsafe {
    (page.start_address() + padr.as_u64() % mem::PAGE_SIZE as u64)
      .as_ptr::<AcpiHeader>()
      .as_ref()
  }
  .unwrap();
  let aligned_size = align_up(table.len as u64, mem::PAGE_SIZE as u64) as usize;

  *pgindex += 1;

  if aligned_size > mem::PAGE_SIZE {
    let rest = aligned_size - mem::PAGE_SIZE;
    mem::vmap(page + 1, frame + 1, rest, PageTableFlags::empty())
      .expect(ERROR_MSG);
    *pgindex += rest / mem::PAGE_SIZE;
  }

  vmemmap.insert(frame, page);
  return table;
}
