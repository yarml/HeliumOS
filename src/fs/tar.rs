use alloc::string::String;
use core::{ascii::Char, ffi::CStr, str::from_utf8};

#[repr(C, packed)]
pub struct TarHeader {
  name: [u8; 100],
  mode: [u8; 8],
  user: [u8; 8],
  group: [u8; 8],
  size: [u8; 12],
  lmod: [u8; 12],
  checksum: [u8; 8],
  ftype: u8,
  link_name: [u8; 100],
  ustar: [u8; 6],
  version: [u8; 2],
  user_name: [u8; 32],
  group_name: [u8; 32],
  dev_major: [u8; 8],
  dev_minor: [u8; 8],
  name_pref: [u8; 155],
  res0: [u8; 12],
}

// Only the ones we support
pub enum TarEntryType {
  File,
  Directory,
}

impl TryFrom<u8> for TarEntryType {
  type Error = ();

  fn try_from(value: u8) -> Result<Self, Self::Error> {
    match value.as_ascii().ok_or(())? {
      Char::Digit0 => Ok(Self::File),
      Char::Digit5 => Ok(Self::Directory),
      _ => Err(()),
    }
  }
}

impl TarHeader {
  pub fn check(&self) -> bool {
    match from_utf8(&self.ustar) {
      Err(_) => false,
      Ok(signature) => signature == "ustar ",
    }
  }
  pub fn name(&self) -> String {
    let pref_len = self.name_pref.iter().position(|b| *b == 0).unwrap_or(155);
    let name_len = self.name.iter().position(|b| *b == 0).unwrap_or(100);

    let mut filename = String::new();

    let pref = from_utf8(&self.name_pref[..pref_len]).unwrap();
    let name = from_utf8(&self.name[..name_len]).unwrap();

    filename.push_str(pref);
    filename.push_str(name);

    filename
  }

  pub fn size(&self) -> usize {
    let size_str = CStr::from_bytes_with_nul(&self.size)
      .unwrap()
      .to_str()
      .unwrap();
    usize::from_str_radix(size_str, 8).unwrap()
  }

  pub fn entry_type(&self) -> Option<TarEntryType> {
    TarEntryType::try_from(self.ftype).ok()
  }
}
