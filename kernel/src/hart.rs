mod glob;

pub use glob::GlobHartInfo;

static GLOB_HART_INFO: GlobHartInfo = GlobHartInfo::new();

pub fn glob_info() -> &'static GlobHartInfo {
  &GLOB_HART_INFO
}
