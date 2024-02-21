use crate::io;

pub fn disable() {
  io::outb(0x20, 0x11);
  io::wait();
  io::outb(0xA0, 0x11);
  io::wait();

  io::outb(0x21, 0x20);
  io::wait();
  io::outb(0xA1, 0x20);
  io::wait();
  io::outb(0x21, 4);
  io::wait();
  io::outb(0xA1, 2);
  io::wait();

  io::outb(0x21, 1);
  io::wait();
  io::outb(0xA1, 1);
  io::wait();

  io::outb(0x21, 0xFF);
  io::outb(0xA1, 0xFF);

  io::wait();
}
