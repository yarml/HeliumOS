use super::tar::{TarEntryType, TarHeader};
use crate::{
  bootboot::bootboot, feat::collections::hashmap::HashMap, mem::vmap, println,
};
use alloc::string::String;
use core::mem;
use spin::Once;
use x86_64::{
  align_up,
  structures::paging::{Page, PageTableFlags, PhysFrame, Size4KiB},
  PhysAddr, VirtAddr,
};

const INITRD_START: Page<Size4KiB> = unsafe {
  Page::from_start_address_unchecked(VirtAddr::new_truncate(0xFFFF810000000000))
};

#[derive(Clone, Copy)]
pub enum InitrdEntry {
  File(InitrdFile),
  Directory,
}

#[derive(Clone, Copy)]
pub struct InitrdFile {
  size: usize,
  content: VirtAddr,
}

static INITRD: Once<HashMap<String, InitrdEntry>> = Once::new();

pub fn init() {
  let bootboot = bootboot();
  let initrd_frame = PhysFrame::<Size4KiB>::from_start_address(PhysAddr::new(
    bootboot.initrd_ptr,
  ))
  .unwrap();

  vmap(
    INITRD_START,
    initrd_frame,
    bootboot.initrd_size as usize,
    PageTableFlags::empty(),
  )
  .unwrap();

  INITRD.call_once(init_initrd_map);
}

fn init_initrd_map() -> HashMap<String, InitrdEntry> {
  let bootboot = bootboot();
  let mut current_header = INITRD_START.start_address().as_ptr::<TarHeader>();

  let mut initrd = HashMap::new();

  // FIXME: Assumes that directories always come before files under them
  loop {
    if (current_header as usize) + mem::size_of::<TarHeader>()
      > INITRD_START.start_address().as_u64() as usize
        + bootboot.initrd_size as usize
    {
      break;
    }

    let ch = unsafe { current_header.as_ref() }.unwrap();
    if !ch.check() {
      break;
    }
    let entry_size = ch.size();
    current_header = ((current_header as u64)
      + mem::size_of::<TarHeader>() as u64
      + align_up(entry_size as u64, 512))
      as *const TarHeader;

    match ch.entry_type() {
      None => continue,
      Some(ftype) => {
        let entry_name = ch.name();
        if entry_name.ends_with(".") || entry_name.ends_with("..") {
          continue;
        }

        let entry = match ftype {
          TarEntryType::File => InitrdEntry::File(InitrdFile {
            size: ch.size(),
            content: VirtAddr::zero(),
          }),
          TarEntryType::Directory => InitrdEntry::Directory,
        };

        println!("Inserting entry {}", &entry_name);
        initrd.insert(entry_name, entry);
      }
    }
  }

  initrd
}
