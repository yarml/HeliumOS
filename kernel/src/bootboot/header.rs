use super::{
  framebuffer::{FramebufferInfo, FramebufferType},
  mmap::{MmapRawEntry, PhysMemoryMap},
};
use core::{ffi, mem, slice};

#[repr(C, packed)]
pub struct Header {
  magic: [u8; 4],
  size: u32,
  protocol: u8,
  fb_type: u8,
  numcores: u16,
  bspid: u16,
  timezone: i16,
  datetime: [u8; 8],
  initrd_ptr: u64,
  initrd_size: u64,
  fb_ptr: u64,
  fb_size: u32,
  fb_width: u32,
  fb_height: u32,
  fb_scanline: u32,
  acpi_ptr: u64,
  smbi_ptr: u64,
  efi_ptr: u64,
  mp_ptr: u64,
  unused0: u64,
  unused1: u64,
  unused2: u64,
  unused3: u64,
}

pub enum Loader {
  Bios,
  Uefi,
  Rpi,
  Coreboot,
  Other,
}

impl Header {
  pub const fn instance() -> &'static Self {
    unsafe {
      // # Safety
      // Symbol bootboot defined in liker.ld and guarenteed not to be null
      // Also guarenteed to be a pointer to a valid, never destroyed, read-only Header
      &super::bootboot
    }
  }
}

impl Header {
  pub fn check(&self) -> bool {
    &self.magic == b"BOOT"
  }

  pub fn loader(&self) -> Loader {
    (self.protocol & 0b1111100).into()
  }

  pub const fn numcores(&self) -> usize {
    self.numcores as usize
  }

  pub const fn bspid(&self) -> usize {
    self.bspid as usize
  }

  pub fn framebuffer(&self) -> FramebufferInfo {
    FramebufferInfo {
      // No Panic: Bootboot should guarentee a valid value
      typ: FramebufferType::try_from(self.fb_type).unwrap(),
      phys_location: self.fb_ptr.into(),
      size: self.fb_size as usize,
      width: self.fb_width as usize,
      height: self.fb_height as usize,
      scanline: self.fb_scanline as usize,
      location: unsafe {
        // # Safety
        // Nothing to worry about
        &super::fb
      } as *const ffi::c_void as *mut u8,
    }
  }

  pub const fn mmap(&self) -> PhysMemoryMap {
    let nentries = (self.size as usize - mem::size_of::<Header>())
      / mem::size_of::<MmapRawEntry>();
    let start = (self as *const Header as *const u8)
      .wrapping_add(mem::size_of::<Header>())
      as *const MmapRawEntry;
    let mmap = unsafe {
      // # Safety
      // - start is not NULL, guarenteed by Bootboot
      // - start is valid for reads for at least len * mem::size_of::<MmapEntry> bytes, guarenteed by Bootboot
      // - MmapEntry is aligned to 1, so any alignment is valid
      // - start contains at least nentries valid MmapEntrys, guarenteed by Bootboot
      // - This memory is never mutably referenced in the kernel
      // - The maximum size of the memory map is 3968 bytes, which is less than isize::MAX, guarenteed by Bootboot
      slice::from_raw_parts(start, nentries)
    };
    PhysMemoryMap::from(mmap)
  }
}

impl From<u8> for Loader {
  fn from(value: u8) -> Self {
    match value {
      0 => Self::Bios,
      1 => Self::Uefi,
      2 => Self::Rpi,
      3 => Self::Coreboot,
      _ => Self::Other,
    }
  }
}
