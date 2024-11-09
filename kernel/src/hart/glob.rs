pub struct GlobHartInfo {
  hart_count: Option<usize>,
}

impl GlobHartInfo {
  pub const fn new() -> Self {
    Self { hart_count: None }
  }

  pub fn hart_count(&self) -> Option<usize> {
    self.hart_count
  }
}