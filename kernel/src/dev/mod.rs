pub mod console;
pub mod framebuffer;

pub fn init() {
  framebuffer::late_init();
}
