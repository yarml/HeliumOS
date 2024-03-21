pub fn init() {
  acpi::init();
}

pub mod acpi {
  use crate::{acpi::tables::AcpiHeader, proc::apic};
  use alloc::{collections::BTreeMap, string::String, vec::Vec};
  use spin::{once::Once, rwlock::RwLock};

  type Handler = fn(&AcpiHeader);

  static HANDLERS: Once<RwLock<BTreeMap<String, Vec<Handler>>>> = Once::new();

  pub(super) fn init() {
    HANDLERS.call_once(|| RwLock::new(BTreeMap::new()));
    register("APIC", apic::cfgtb::acpi_handler);
  }

  pub fn register(signature: &str, handler: Handler) {
    let mut handlers_lock = HANDLERS.get().unwrap().write();

    let list = match handlers_lock.get_mut(signature) {
      None => {
        handlers_lock.insert(String::from(signature), Vec::new());
        handlers_lock.get_mut(signature).unwrap()
      }
      Some(list) => list,
    };

    list.push(handler);
  }

  pub fn call(signature: &str, table: &AcpiHeader) -> bool {
    let handler_lock = HANDLERS.get().unwrap().read();
    match handler_lock.get(signature) {
      Some(list) => {
        for handler in list {
          handler(table);
        }

        list.len() != 0
      }
      _ => false,
    }
  }
}
