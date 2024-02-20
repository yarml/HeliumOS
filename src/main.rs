#![no_std]
#![no_main]

#[allow(dead_code)]
#[allow(non_snake_case)]
#[allow(non_camel_case_types)]
mod bootboot;

// Required for -Z build-std flag.
extern crate rlibc;

/******************************************
 * Entry point, called by BOOTBOOT Loader *
 ******************************************/
#[no_mangle] // don't mangle the name of this function
fn _start() -> ! {
  /*** NOTE: this code runs on all cores in parallel ***/
  use bootboot::*;

  //Lets use the BOOTBOOT_INFO as a pointer, dereference it and immediately borrow it.
  let bootboot_r = unsafe { &(*(BOOTBOOT_INFO as *const BOOTBOOT)) };

  if bootboot_r.fb_scanline > 0 {
    //As pointer arithmetic is not possible in rust, use the address as u64
    let fb = BOOTBOOT_FB as u64;

    // cross-hair to see screen dimension detected correctly
    for y in 0..bootboot_r.fb_height {
      let addr = fb
        + bootboot_r.fb_scanline as u64 * y as u64
        + bootboot_r.fb_width as u64 * 2;
      unsafe { *(addr as *mut u64) = 0x00FFFFFF };
    }
    for x in 0..bootboot_r.fb_width {
      let addr = fb
        + bootboot_r.fb_scanline as u64 * (bootboot_r.fb_height / 2) as u64
        + (x * 4) as u64;
      unsafe { *(addr as *mut u64) = 0x00FFFFFF };
    }

    // red, green, blue boxes in order
    for y in 0..20 {
      for x in 0..20 {
        let addr =
          fb + bootboot_r.fb_scanline as u64 * (y + 20) as u64 + (x + 20) * 4;
        unsafe { *(addr as *mut u64) = 0x00FF0000 };
      }
    }
    for y in 0..20 {
      for x in 0..20 {
        let addr =
          fb + bootboot_r.fb_scanline as u64 * (y + 20) as u64 + (x + 50) * 4;
        unsafe { *(addr as *mut u64) = 0x0000FF00 };
      }
    }
    for y in 0..20 {
      for x in 0..20 {
        let addr =
          fb + bootboot_r.fb_scanline as u64 * (y + 20) as u64 + (x + 80) * 4;
        unsafe { *(addr as *mut u64) = 0x000000FF };
      }
    }
  }

  // hang for now
  loop {}
}

#[panic_handler]
fn panic(_info: &core::panic::PanicInfo) -> ! {
  loop {}
}
