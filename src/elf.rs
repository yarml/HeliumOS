use core::slice;
use x86_64::{structures::paging::PageTableFlags, VirtAddr};

#[repr(C, packed)]
pub struct Identification {
  magic: [u8; 4],
  class: Class,
  endian: Endian,
  version: u8,
  osabi: u8,
  abiver: u8,
  pad: [u8; 7],
}

impl Identification {
  pub fn verify(&self) -> bool {
    &self.magic == b"\x7fELF"
      && self.class == Class::Class64
      && self.endian == Endian::LSB
      && self.version == 1
  }
}

#[repr(u8)]
#[derive(PartialEq)]
pub enum Class {
  None,
  Class32,
  Class64,
}

#[repr(u8)]
#[derive(PartialEq)]
pub enum Endian {
  None,
  LSB,
  MSB,
}

pub struct ElfFile<'a> {
  pub header: &'a FileHeader,
  pub size: usize,
}
impl<'a> ElfFile<'a> {
  pub unsafe fn parse(content: &'a [u8]) -> Option<ElfFile> {
    let r = (content as *const [u8] as *const FileHeader)
      .as_ref()
      .unwrap();
    if !r.verify() {
      None
    } else {
      Some(Self {
        header: r,
        size: content.len(),
      })
    }
  }
  pub fn slice(&self, offset: usize, size: usize) -> Option<&[u8]> {
    if offset + size > self.size {
      None
    } else {
      let ptr =
        unsafe { (self.header as *const FileHeader as *const u8).add(offset) };
      Some(unsafe { slice::from_raw_parts(ptr, size) })
    }
  }
}

#[repr(C, packed)]
pub struct FileHeader {
  id: Identification,
  ftype: FileType,
  machine: FileMachine,
  version: u32,
  entry: u64,
  phoff: u64,
  shoff: u64,
  flags: u32,
  ehsize: u16,
  phent_size: u16,
  pht_len: u16,
  shent_size: u16,
  sht_len: u16,
  shstridx: u16,
}

impl FileHeader {
  pub fn verify(&self) -> bool {
    self.id.verify()
      && self.version == 1
      && VirtAddr::try_new(self.entry).is_ok()
  }

  pub fn entrypoint(&self) -> VirtAddr {
    VirtAddr::new_truncate(self.entry)
  }

  pub fn iter_progheader(&self) -> ProgramHeaderIterator {
    ProgramHeaderIterator {
      file: self,
      index: 0,
    }
  }
}

pub struct ProgramHeaderIterator<'a> {
  file: &'a FileHeader,
  index: usize,
}

impl<'a> Iterator for ProgramHeaderIterator<'a> {
  type Item = &'a ProgramHeader;

  fn next(&mut self) -> Option<Self::Item> {
    if self.index >= self.file.pht_len as usize {
      return None;
    }
    let current = unsafe {
      ((self.file as *const FileHeader as usize
        + self.file.phoff as usize
        + self.index * self.file.phent_size as usize)
        as *const ProgramHeader)
        .as_ref()
    }
    .unwrap();

    self.index += 1;

    Some(current)
  }
}

#[repr(u16)]
pub enum FileType {
  None,
  Relocatable,
  Executable,
  Shared,
  Core,
}

#[repr(u16)]
pub enum FileMachine {
  None,
  M32,
  Sparc,
  I386,
  K68,
  K88,
  I860 = 7,
  Mips3,
  Mips4 = 10,
}

#[repr(C)]
pub struct ProgramHeader {
  pub ptype: ProgramType,
  flags: u32,
  pub offset: u64,
  pub vadr: u64,
  pub padr: u64,
  pub file_size: u64,
  pub mem_size: u64,
  pub align: u64,
}

impl ProgramHeader {
  pub fn executable(&self) -> bool {
    (self.flags & 1) != 0
  }
  pub fn writable(&self) -> bool {
    (self.flags & 2) != 0
  }
  pub fn readable(&self) -> bool {
    (self.flags & 4) != 0
  }

  pub fn memory_flags(&self) -> PageTableFlags {
    let mut flags = PageTableFlags::empty();

    if self.writable() {
      flags |= PageTableFlags::WRITABLE;
    }

    flags
  }
}

#[repr(u32)]
#[derive(Debug, Clone, Copy)]
pub enum ProgramType {
  Null,
  Load,
  Dynamic,
  Interpreter,
  Note,
  ShLib,
  ProgHeader,
}
