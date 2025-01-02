pub struct Page4KiB;
pub struct Page2MiB;
pub struct Page1GiB;

pub trait PageSize {
  const SHIFT: usize;
  const SIZE: usize = 1 << Self::SHIFT;
  const MASK: usize = usize::MAX >> Self::SHIFT << Self::SHIFT;
}

impl PageSize for Page4KiB {
  const SHIFT: usize = 12;
}

impl PageSize for Page2MiB {
  const SHIFT: usize = 21;
}

impl PageSize for Page1GiB {
  const SHIFT: usize = 30;
}
