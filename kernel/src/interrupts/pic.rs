use x86_64::instructions::port::PortWriteOnly;

use crate::io;

const PIC_MASTER_PORT_CMD: u16 = 0x20;
const PIC_MASTER_PORT_DATA: u16 = 0x21;

const PIC_SLAVE_PORT_CMD: u16 = 0xA0;
const PIC_SLAVE_PORT_DATA: u16 = 0xA1;

pub fn disable() {
  let mut master_command = PortWriteOnly::<u8>::new(PIC_MASTER_PORT_CMD);
  let mut master_data = PortWriteOnly::<u8>::new(PIC_MASTER_PORT_DATA);

  let mut slave_command = PortWriteOnly::<u8>::new(PIC_SLAVE_PORT_CMD);
  let mut slave_data = PortWriteOnly::<u8>::new(PIC_SLAVE_PORT_DATA);

  unsafe {
    master_command.write(0x11);
    io::wait();
    slave_command.write(0x11);
    io::wait();

    master_data.write(0x20);
    io::wait();
    slave_data.write(0x20);
    io::wait();
    master_data.write(4);
    io::wait();
    slave_data.write(2);
    io::wait();

    master_data.write(1);
    io::wait();
    slave_data.write(1);
    io::wait();

    master_data.write(0xFF);
    slave_data.write(0xFF);

    io::wait();
  }
}
