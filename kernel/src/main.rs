#![no_std]
#![no_main]
#![feature(ptr_as_ref_unchecked)]
#![feature(ascii_char)]
#![deny(unsafe_op_in_unsafe_fn)]

extern crate alloc;

mod bootboot;
mod dev;
mod error;
mod hart;
mod io;
mod logging;
mod mem;
mod start;
mod sync;
mod system;
