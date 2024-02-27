use core::{borrow::BorrowMut, mem, slice};

use crate::mem::{
  virt::{
    self, alloc_substruct,
    vcache::{self, VCACHE},
    KVMSPACE,
  },
  PAGE_SIZE,
};
use spin::{Once, RwLock};
use x86_64::{
  align_up,
  registers::control::Cr3,
  structures::paging::{
    mapper::{MapperFlush, MapperFlushAll},
    page_table::PageTableEntry,
    Page, PageTable, PageTableFlags, PhysFrame, Size4KiB,
  },
  PhysAddr,
};

use super::vcache::VCache;

pub(in crate::mem::virt) fn init() {
  // All the mapper needs is mapping the P4 table into virtual memory
  let (p4_frame, _) = Cr3::read();
  let mut vcache = VCACHE.get().unwrap().write();
  let page = vcache.map(p4_frame).unwrap();

  P4TABLE.call_once(|| {
    RwLock::new(
      unsafe { page.start_address().as_mut_ptr::<PageTable>().as_mut() }
        .unwrap(),
    )
  });
}

pub(in crate::mem::virt) fn remove_identity() {
  let p4table = P4TABLE.get().unwrap().write();
  let p4table_raw = unsafe {
    slice::from_raw_parts_mut(
      *p4table as *const PageTable as *mut u8,
      PAGE_SIZE,
    )
  };
  p4table_raw
    .split_at_mut(256 * mem::size_of::<PageTableEntry>())
    .0
    .fill(0);
  MapperFlushAll::new().flush_all();
}

static P4TABLE: Once<RwLock<&mut PageTable>> = Once::new();
pub static MAPPER: RwLock<KernelMapper> = RwLock::new(KernelMapper);

const RLCR3_THRESHOLD: usize = 32;

pub struct KernelMapper;

#[derive(Debug)]
pub enum KernelMapError {
  NullSize,
  Managed,
  NoVCacheSpace,
}

impl KernelMapper {
  pub fn map(
    &self,
    page: Page<Size4KiB>,
    phys: PhysFrame<Size4KiB>,
    size: usize,
    mut flags: PageTableFlags,
  ) -> Result<(), KernelMapError> {
    if size == 0 {
      return Err(KernelMapError::NullSize);
    }

    // check that address is not managed by another kernel subsystem
    if vcache::START_ADDR <= page.start_address()
      && page.start_address() < vcache::START_ADDR + vcache::SIZE
    {
      return Err(KernelMapError::Managed);
    }

    // Align size to page size
    let size = align_up(size as u64, PAGE_SIZE as u64) as usize;
    let num_pages = size / PAGE_SIZE;

    // Flush virtual tables using invlpg if:
    //  - the mapping is explicitly global
    //  - OR the mapping is in kernel space(implicitly global)
    //  - OR the total number of pages to map is less than the rlcr3
    //       threshold.
    let detail_flush = flags.contains(PageTableFlags::GLOBAL)
      || page.start_address() >= virt::KVMSPACE
      || size / PAGE_SIZE < RLCR3_THRESHOLD;

    let mut vcache = VCACHE.get().unwrap().write();

    let tmp_pages = {
      let null_frame = PhysFrame::containing_address(PhysAddr::zero());
      match vcache.map_many(&[null_frame, null_frame]) {
        Ok(pages) => pages,
        Err(_) => return Err(KernelMapError::NoVCacheSpace),
      }
    };

    let mut p4table = P4TABLE.get().unwrap().write();

    for i in 0..num_pages {
      let page = Page::<Size4KiB>::from_start_address(
        page.start_address() + i * PAGE_SIZE,
      )
      .unwrap();
      let physframe = PhysFrame::<Size4KiB>::from_start_address(
        phys.start_address() + i * PAGE_SIZE,
      )
      .unwrap();

      let p4_entry = &mut p4table[page.p4_index()];

      let vc = vcache.borrow_mut();

      // Poor borrow checker has no clue of the tomfoolery we do here
      // Hopefully it doesn't get optimized away...
      let p3 = Self::ensure_allocated(p4_entry, page, vc, tmp_pages[0]);
      let p3_entry = &mut p3[page.p3_index()];

      let p2 = Self::ensure_allocated(p3_entry, page, vc, tmp_pages[1]);
      let p2_entry = &mut p2[page.p2_index()];

      let p1 = Self::ensure_allocated(p2_entry, page, vc, tmp_pages[0]);

      let p1_entry = &mut p1[page.p1_index()];

      // No matter the whims of the callee, an address in kernel space is
      // Global and user inaccessible
      if page.start_address() > KVMSPACE {
        flags |= PageTableFlags::GLOBAL;
        flags &= !PageTableFlags::USER_ACCESSIBLE;
      }

      // And not matter the whimes of the callee, a mapping is present
      flags |= PageTableFlags::PRESENT;
      p1_entry.set_frame(physframe, flags);

      if detail_flush {
        MapperFlush::new(page).flush();
      }
    }

    vcache.unmap_many(&tmp_pages);

    if !detail_flush {
      MapperFlushAll::new().flush_all();
    }

    Ok(())
  }

  fn ensure_allocated(
    entry: &mut PageTableEntry,
    page: Page<Size4KiB>,
    vcache: &mut VCache,
    map_to_page: Page<Size4KiB>,
  ) -> &'static mut PageTable {
    if entry.is_unused() {
      let mut flags = PageTableFlags::WRITABLE | PageTableFlags::PRESENT;
      if page.start_address() < KVMSPACE {
        flags |= PageTableFlags::USER_ACCESSIBLE;
      }
      alloc_substruct(entry, flags);
    }

    let frame = entry.frame().unwrap();

    vcache.remap(map_to_page, frame).unwrap();

    unsafe {
      map_to_page
        .start_address()
        .as_mut_ptr::<PageTable>()
        .as_mut()
    }
    .unwrap()
  }
}
