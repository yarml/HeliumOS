//! # Middle Memory
//! Any usable memory in the range [16Mib 4Gib).
//! For DMA devices capable of it, and general purpose applications
//! when high memory cannot fullfill a request
//!
//! # Supported Requests
//! - Class 0: 4Kib on 4Kib boundary
//! - Class 4: 64Kib on 64Kib boundary
//! - Class 5: 128Kib on 128Kib boundary
//! - Class 9: 2Mib on 2Mib boundary
//!
//! # Kernel Space
//! Like low memory, this entire memory space can be found in kernel space
//! with constant offset using PhysAddr::to_virt()

struct MiddleMemoryAllocator {
  
}
