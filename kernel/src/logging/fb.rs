use crate::{dev::fb::Framebuffer, sync::Mutex};

pub static FRAMEBUFFER: Mutex<Framebuffer> = Mutex::new(Framebuffer::default());

