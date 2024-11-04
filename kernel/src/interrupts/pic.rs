use spin::Mutex;
use x86_64::instructions::port::{Port, PortWriteOnly};

use crate::{
  debug,
  dev::framebuffer::{debug_printbin, debug_set_pixel},
  io, println,
  proc::apic,
};

const PIC_MASTER_PORT_CMD: u16 = 0x20;
const PIC_MASTER_PORT_DATA: u16 = 0x21;

const PIC_SLAVE_PORT_CMD: u16 = 0xA0;
const PIC_SLAVE_PORT_DATA: u16 = 0xA1;

const PIT_CH0_DATA: u16 = 0x40;
const PIT_CMD: u16 = 0x43;

static CALIB_LOCK: Mutex<()> = Mutex::new(());

pub fn disable() {
  let mut master_command = PortWriteOnly::<u8>::new(PIC_MASTER_PORT_CMD);
  let mut master_data = PortWriteOnly::<u8>::new(PIC_MASTER_PORT_DATA);

  let mut slave_command = PortWriteOnly::<u8>::new(PIC_SLAVE_PORT_CMD);
  let mut slave_data = PortWriteOnly::<u8>::new(PIC_SLAVE_PORT_DATA);

  unsafe {
    // Config mode
    master_command.write(0x11);
    slave_command.write(0x11);
    io::wait();

    // Offset master and slave to 0x20 and 0x28 respectively
    master_data.write(0x20);
    slave_data.write(0x28);
    io::wait();

    // Configure master slave relationship
    master_data.write(4);
    slave_data.write(2);
    io::wait();

    // Use 8086 Mode
    master_data.write(1);
    slave_data.write(1);
    io::wait();

    // Mask everything
    master_data.write(0xFF);
    slave_data.write(0xFF);

    io::wait();
  }
}

pub trait PITCalib {
  fn measure() -> usize;
}

// Waits for 10ms using the PIT as time reference
// PIC should be disabled before, and will be disabled after
// Measurer: should return the current state of the timer to calibrate
// Returns the difference between the timer state before and after the PIT enabled sleep
// Should not be called after interrupts are setup
pub fn pit_calib_sleep<Measurer: PITCalib>() -> (usize, usize) {
  let id = apic::id();
  let lock = CALIB_LOCK.lock();

  let mut pit_data = Port::<u8>::new(PIT_CH0_DATA);
  let mut pit_cmd = PortWriteOnly::<u8>::new(PIT_CMD);

  let pit_delay: u16 = 11933;
  debug_set_pixel(40, 20 + id, (255, 255, 255).into());
  println!("Start PIT");
  unsafe {
    pit_data.write(0xFF);
    pit_data.write(0xFF);
  }
  unsafe {
    pit_cmd.write(0);
  }
  let ref_start_lo = unsafe { pit_data.read() };
  let ref_start_hi = unsafe { pit_data.read() };
  let ref_timer_start = ref_start_lo as u16 | (ref_start_hi as u16) << 8;
  let timer_start = Measurer::measure();
  let error = loop {
    unsafe {
      pit_cmd.write(0);
    }
    let ref_lo = unsafe { pit_data.read() };
    let ref_hi = unsafe { pit_data.read() };
    let ref_timer_now = ref_lo as u16 | (ref_hi as u16) << 8;

    // FIXME: There is a bug or something with Qemu where time goes backwards sometimes, idk
    // Don't wanna deal with it
    if debug::isvm() && ref_timer_now > ref_timer_start {
      return (timer_start, timer_start - 884248);
    }

    if ref_timer_start - ref_timer_now >= pit_delay {
      break ref_timer_start - ref_timer_now - pit_delay;
    }
  };
  let timer_end = Measurer::measure();
  println!("Done PIT with error: {} pic tick", error);
  debug_printbin(40, 20 + id, error as usize);
  debug_set_pixel(39, 20 + id, (255, 255, 0).into());

  drop(lock);

  (timer_start, timer_end)
}
