use super::{valloc_ktable, virt::KVMSPACE};
use crate::proc::apic::{self, numcores};
use lazy_static::lazy_static;
use x86_64::{
  instructions::tables,
  registers::segmentation::{Segment, CS, DS, ES, FS, GS, SS},
  structures::{
    gdt::{Descriptor, GlobalDescriptorTable, SegmentSelector},
    tss::TaskStateSegment,
  },
  VirtAddr,
};

static GDT_TABLE_PTR: VirtAddr =
  VirtAddr::new_truncate(KVMSPACE.as_u64() + 5 * 1024 * 1024 * 1024 * 1024);
static TSS_TABLE_PTR: VirtAddr =
  VirtAddr::new_truncate(KVMSPACE.as_u64() + 7 * 1024 * 1024 * 1024 * 1024);
lazy_static! {
  static ref BASIC_GDT: BasicGlobalDescriptorTable = {
    let mut gdt = GlobalDescriptorTable::new();
    let code_segment = gdt.append(Descriptor::kernel_code_segment());
    let data_segment = gdt.append(Descriptor::kernel_data_segment());

    BasicGlobalDescriptorTable {
      gdt,
      code_segment,
      data_segment,
    }
  };
}

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

pub(in crate::mem) fn basic_init() {
  BASIC_GDT.load();
}

pub struct KernelGlobalDescriptorTable {
  gdt: GlobalDescriptorTable,
  pub kcode_seg: SegmentSelector,
  pub kdata_seg: SegmentSelector,
  pub ucode_seg: SegmentSelector,
  pub udata_seg: SegmentSelector,
  tss_seg: SegmentSelector,
}

impl KernelGlobalDescriptorTable {
  pub fn init() {
    let numcores = numcores();
    // Allocate & Map GDT_TABLE
    valloc_ktable::<KernelGlobalDescriptorTable>(GDT_TABLE_PTR, numcores);

    // Allocate & Map TSS_TABLE
    valloc_ktable::<TaskStateSegment>(TSS_TABLE_PTR, numcores);
  }

  // unsafe: The caller must verify that init() was called before
  pub unsafe fn entry() -> &'static mut KernelGlobalDescriptorTable {
    let ptr = GDT_TABLE_PTR
      .as_mut_ptr::<KernelGlobalDescriptorTable>()
      .add(apic::id());
    ptr.as_mut().unwrap()
  }

  // unsafe: The caller must ensure that init() was called before
  pub unsafe fn tss() -> &'static mut TaskStateSegment {
    let ptr = GDT_TABLE_PTR
      .as_mut_ptr::<TaskStateSegment>()
      .add(apic::id());
    ptr.as_mut().unwrap()
  }

  // Unsafe , the caller should make sure init() was called before
  pub unsafe fn register(
    kstack: VirtAddr,
    nmistack: VirtAddr,
    dfstack: VirtAddr,
  ) {
    let mut gdt = GlobalDescriptorTable::new();
    let kcode_seg = gdt.append(Descriptor::kernel_code_segment());
    let kdata_seg = gdt.append(Descriptor::kernel_data_segment());
    let udata_seg = gdt.append(Descriptor::user_data_segment());
    let ucode_seg = gdt.append(Descriptor::user_code_segment());

    let tss = Self::tss();
    *tss = TaskStateSegment::new();
    tss.privilege_stack_table[0] = kstack;
    tss.interrupt_stack_table[0] = nmistack;
    tss.interrupt_stack_table[1] = dfstack;

    let tss_seg = gdt.append(Descriptor::tss_segment(tss));

    // We don't load the GDT just yet, until it is in its final location
    let entry = unsafe { Self::entry() };
    *entry = Self {
      gdt,
      kcode_seg,
      kdata_seg,
      ucode_seg,
      udata_seg,
      tss_seg,
    };

    entry.gdt.load(); // Now we can load the GDT
    entry.load_kernel();
  }

  // unsafe: Caller has to verify that the currently
  // loaded GDT is the one managed by this struct
  unsafe fn load_kernel(&self) {
    CS::set_reg(self.kcode_seg);
    SS::set_reg(self.kdata_seg);
    DS::set_reg(self.kdata_seg);
    ES::set_reg(self.kdata_seg);
    FS::set_reg(self.kdata_seg);
    GS::set_reg(self.kdata_seg);
    tables::load_tss(self.tss_seg);
  }
}
