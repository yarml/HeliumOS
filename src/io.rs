use x86_64::instructions::port::PortWriteOnly;

pub fn wait() {
  let mut tmp_port = PortWriteOnly::<u8>::new(0x80);
  unsafe { tmp_port.write(0) };
}
