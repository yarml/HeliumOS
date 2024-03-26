use super::tar::{TarEntryType, TarHeader};
use crate::{bootboot::bootboot, mem::vmap};
use alloc::{collections::BTreeMap, string::String};
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
  File(&'static [u8]),
  Directory,
}

static INITRD: Once<BTreeMap<String, InitrdEntry>> = Once::new();

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

  INITRD.call_once(init_map);
}

pub fn open(path: &str) -> Option<&InitrdEntry> {
  match INITRD.get() {
    None => panic!("Initrd not initialized yet and tried to open {}", path),
    Some(initrd) => initrd.get(path),
  }
}
pub fn open_file(path: &str) -> Option<&[u8]> {
  match open(path) {
    None => None,
    Some(entry) => match entry {
      InitrdEntry::File(file) => Some(file),
      InitrdEntry::Directory => None,
    },
  }
}

fn init_map() -> BTreeMap<String, InitrdEntry> {
  let bootboot = bootboot();
  let mut current_header = INITRD_START.start_address().as_ptr::<TarHeader>();

  let mut initrd = BTreeMap::new();

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
          TarEntryType::File => InitrdEntry::File(ch.content()),
          TarEntryType::Directory => InitrdEntry::Directory,
        };
        initrd.insert(entry_name, entry);
      }
    }
  }

  initrd
}
