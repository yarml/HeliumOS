#![no_std]
#![no_main]
#![feature(ptr_as_ref_unchecked)]
#![feature(ascii_char)]
#![feature(maybe_uninit_slice)]
#![feature(deref_pure_trait)]
#![feature(integer_atomics)]
#![deny(unsafe_op_in_unsafe_fn)]

extern crate alloc;

mod bootboot;
mod collections;
mod dev;
mod error;
mod hart;
mod io;
mod logging;
mod mem;
mod start;
mod sync;
mod system;
