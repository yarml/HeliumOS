use core::sync::atomic::{AtomicBool, Ordering};

use spin::Mutex;
use x86_64::{
  instructions::{interrupts, port::PortWriteOnly},
  structures::idt::InterruptStackFrame,
};

use crate::{io, println};

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

static PIT_CALIB_DONE: AtomicBool = AtomicBool::new(false);
extern "x86-interrupt" fn pit_interrupt(_frame: InterruptStackFrame) {
  PIT_CALIB_DONE.store(true, Ordering::Release);
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
  let lock = CALIB_LOCK.lock();

  let mut pit_data = PortWriteOnly::<u8>::new(PIT_CH0_DATA);
  let mut pit_cmd = PortWriteOnly::<u8>::new(PIT_CMD);
  let mut master_data = PortWriteOnly::<u8>::new(PIC_MASTER_PORT_DATA);

  super::setup_pit_calib(pit_interrupt);

  let pit_delay: u16 = 11933;

  let pit_delay_lo = (pit_delay & 0xFF) as u8;
  let pit_delay_hi = ((pit_delay >> 8) & 0xFF) as u8;

  unsafe {
    // Setup PIT channel 0 in Mode 0(One shot)
    pit_cmd.write(0b00110000);
    // Remove PIC mask
    master_data.write(0xFE);
    // Write large value here so we don't get interrupted immediatly after enabling interrupts
    pit_data.write(0xFF);
    pit_data.write(0xFF);
  }
  interrupts::enable();
  unsafe {
    pit_data.write(pit_delay_lo);
    pit_data.write(pit_delay_hi);
    pit_cmd.write(0b00110000);
  }
  PIT_CALIB_DONE.store(true, Ordering::SeqCst);
  let timer_start = Measurer::measure();
  while !PIT_CALIB_DONE.fetch_or(false, Ordering::SeqCst) {}
  let timer_end = Measurer::measure();
  interrupts::disable();
  unsafe {
    // Remask PIC
    master_data.write(0xFF);
  }
  println!("Done PIT");

  drop(lock);

  (timer_start, timer_end)
}
