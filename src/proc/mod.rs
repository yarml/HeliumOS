use crate::bootboot::bootboot;

pub mod apic;

pub fn is_primary() -> bool {
  apic::id() == bootboot().bspid
}