use lazy_static::lazy_static;
use x86_64::{
  registers::segmentation::{Segment, CS, DS, ES, FS, GS, SS},
  structures::gdt::{Descriptor, GlobalDescriptorTable, SegmentSelector},
};

struct BasicGlobalDescriptorTable {
  gdt: GlobalDescriptorTable,
  code_segment: SegmentSelector,
  data_segment: SegmentSelector,
}

impl BasicGlobalDescriptorTable {
  fn load(&'static self) {
    self.gdt.load();
    unsafe {
      CS::set_reg(self.code_segment);
      SS::set_reg(self.data_segment);
      DS::set_reg(self.data_segment);
      ES::set_reg(self.data_segment);
      FS::set_reg(self.data_segment);
      GS::set_reg(self.data_segment);
    }
  }
}

lazy_static! {
  static ref BASIC_GDT: BasicGlobalDescriptorTable = {
    let mut gdt = GlobalDescriptorTable::new();
    let code_segment = gdt.add_entry(Descriptor::kernel_code_segment());
    let data_segment = gdt.add_entry(Descriptor::kernel_data_segment());

    BasicGlobalDescriptorTable {
      gdt,
      code_segment,
      data_segment,
    }
  };
}

pub(in crate::mem) fn basic_init() {
  BASIC_GDT.load();
}
