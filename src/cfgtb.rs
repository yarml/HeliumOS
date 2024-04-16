pub fn init() {
  acpi::init();
}

pub mod acpi {
  use crate::{acpi::tables::AcpiHeader, proc::apic};
  use alloc::{collections::BTreeMap, vec::Vec};
  use spin::{once::Once, rwlock::RwLock};

  type Handler = fn(&AcpiHeader);

  static HANDLERS: Once<RwLock<BTreeMap<Vec<u8>, Vec<Handler>>>> = Once::new();

  pub(super) fn init() {
    HANDLERS.call_once(|| RwLock::new(BTreeMap::new()));
    register(b"APIC", apic::cfgtb::acpi_handler);
  }

  pub fn register(signature: &[u8], handler: Handler) {
    let mut handlers_lock = HANDLERS.get().unwrap().write();

    let list = match handlers_lock.get_mut(signature) {
      None => {
        handlers_lock.insert(Vec::from(signature), Vec::new());
        handlers_lock.get_mut(signature).unwrap()
      }
      Some(list) => list,
    };

    list.push(handler);
  }

  pub fn call(signature: &[u8], table: &AcpiHeader) -> bool {
    let handler_lock = HANDLERS.get().unwrap().read();
    match handler_lock.get(signature) {
      Some(list) => {
        for handler in list {
          handler(table);
        }

        !list.is_empty()
      }
      _ => false,
    }
  }
}
