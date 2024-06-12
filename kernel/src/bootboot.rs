use x86_64::{align_down, align_up, PhysAddr, VirtAddr};

use crate::mem::PAGE_SIZE;

pub const BOOTBOOT_MAGIC: &[u8; 5usize] = b"BOOT\0";
pub const PROTOCOL_MINIMAL: u32 = 0;
pub const PROTOCOL_STATIC: u32 = 1;
pub const PROTOCOL_DYNAMIC: u32 = 2;
pub const PROTOCOL_BIGENDIAN: u32 = 128;
pub const LOADER_BIOS: u32 = 0;
pub const LOADER_UEFI: u32 = 4;
pub const LOADER_RPI: u32 = 8;
pub const FB_ARGB: u32 = 0;
pub const FB_RGBA: u32 = 1;
pub const FB_ABGR: u32 = 2;
pub const FB_BGRA: u32 = 3;
pub const MMAP_USED: u32 = 0;
pub const MMAP_FREE: u32 = 1;
pub const MMAP_ACPI: u32 = 2;
pub const MMAP_MMIO: u32 = 3;
pub const INITRD_MAXSIZE: u32 = 16;

pub const BOOTBOOT_MMIO: u64 = 0xfffffffff8000000;
pub const BOOTBOOT_FB: u64 = 0xfffffffffc000000;
pub const BOOTBOOT_INFO: u64 = 0xffffffffffe00000;
pub const BOOTBOOT_ENV: u64 = 0xffffffffffe01000;
pub const BOOTBOOT_CORE: u64 = 0xffffffffffe02000;

extern "C" {
  static initstack: usize;
  static fb: usize;
}

#[repr(C, packed)]
#[derive(Debug, Copy, Clone)]
pub struct MMapEnt {
  pub ptr: u64,
  pub size: u64,
}

#[repr(C, packed)]
#[derive(Copy, Clone)]
pub struct BootBoot {
  pub magic: [u8; 4usize],
  pub size: u32,
  pub protocol: u8,
  pub fb_type: u8,
  pub numcores: u16,
  pub bspid: u16,
  pub timezone: i16,
  pub datetime: [u8; 8usize],
  pub initrd_ptr: u64,
  pub initrd_size: u64,
  pub fb_ptr: *mut u8,
  pub fb_size: u32,
  pub fb_width: u32,
  pub fb_height: u32,
  pub fb_scanline: u32,
  pub arch: ArchInfo,
  pub mmap: MMapEnt,
}

#[repr(C)]
#[derive(Copy, Clone)]
pub union ArchInfo {
  pub x86_64: Archx86,
  pub aarch64: ArchArm64,
  _bindgen_union_align: [u64; 8usize],
}

#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct Archx86 {
  pub acpi_ptr: u64,
  pub smbi_ptr: u64,
  pub efi_ptr: u64,
  pub mp_ptr: u64,
  pub unused0: u64,
  pub unused1: u64,
  pub unused2: u64,
  pub unused3: u64,
}

#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct ArchArm64 {
  pub acpi_ptr: u64,
  pub mmio_ptr: u64,
  pub efi_ptr: u64,
  pub unused0: u64,
  pub unused1: u64,
  pub unused2: u64,
  pub unused3: u64,
  pub unused4: u64,
}

#[repr(C, packed)]
#[derive(Debug, Copy, Clone)]
pub struct PSF2 {
  pub magic: u32,
  pub version: u32,
  pub headersize: u32,
  pub flags: u32,
  pub numglyph: u32,
  pub bytesperglyph: u32,
  pub height: u32,
  pub width: u32,
  pub glyphs: u8,
}

pub fn bootboot() -> &'static BootBoot {
  unsafe { (BOOTBOOT_INFO as *const BootBoot).as_ref().unwrap() }
}
pub fn kernel_stack_size() -> usize {
  unsafe { &initstack as *const usize as usize }
}

pub fn fb_virt() -> VirtAddr {
  VirtAddr::new(unsafe { &fb as *const usize as u64 }.into())
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord)]
pub enum MMapType {
  Used,
  Free,
  Acpi,
  MemoryMappedIO,
}

impl MMapEnt {
  pub fn get_type(&self) -> MMapType {
    match self.size & 0xF {
      0 => MMapType::Used,
      1 => MMapType::Free,
      2 => MMapType::Acpi,
      3 => MMapType::MemoryMappedIO,
      other => panic!("Unknown memory map type {other}"),
    }
  }

  pub fn phyadr(&self) -> PhysAddr {
    PhysAddr::new(align_up(self.ptr, PAGE_SIZE as u64))
  }
  pub fn size(&self) -> usize {
    align_down(self.raw_size() as u64, PAGE_SIZE as u64) as usize
  }
  pub fn raw_size(&self) -> usize {
    self.size as usize & 0xFFFFFFFFFFFFFFF0
  }
}
