use super::{alloc_substruct, KVMSPACE};
use crate::{
  dev::framebuffer::debug_set_pixel,
  mem::{early_heap::EarlyAllocator, PAGE_SIZE},
};
use alloc::{boxed::Box, slice};
use core::mem;
use spin::{rwlock::RwLock, Once};
use x86_64::{
  registers::control::Cr3,
  structures::paging::{
    mapper::MapperFlush, page_table::PageTableEntry, Page, PageTable,
    PageTableFlags, PhysFrame, Size4KiB,
  },
  VirtAddr,
};

pub const START_ADDR: VirtAddr = KVMSPACE;
const P1PAGE_COUNT: usize = 2048;
const P2PAGE_COUNT: usize = P1PAGE_COUNT / 512;
pub const SIZE: usize = P1PAGE_COUNT * PAGE_SIZE;

pub(in crate::mem::virt) fn init() {
  let (p4_frame, _) = Cr3::read();
  let p4 =
    unsafe { (p4_frame.start_address().as_u64() as *mut PageTable).as_mut() }
      .unwrap();
  let p4_entry = &mut p4[START_ADDR.p4_index()];
  if p4_entry.is_unused() {
    alloc_substruct(
      p4_entry,
      PageTableFlags::WRITABLE | PageTableFlags::PRESENT,
      true,
    );
  }

  let p3 = unsafe {
    (p4_entry.frame().unwrap().start_address().as_u64() as *mut PageTable)
      .as_mut()
  }
  .unwrap();

  let p3_entry = &mut p3[START_ADDR.p3_index()];

  if p3_entry.is_unused() {
    alloc_substruct(
      p3_entry,
      PageTableFlags::WRITABLE | PageTableFlags::PRESENT,
      true,
    );
  }

  let p2_frame = p3_entry.frame().unwrap();
  let p2 =
    unsafe { (p2_frame.start_address().as_u64() as *mut PageTable).as_mut() }
      .unwrap();

  {
    for i in 0..P2PAGE_COUNT {
      let p2_entry = &mut p2[i];
      if !p2_entry.is_unused() {
        panic!("VCache target P2 page entries found in invalid state");
      }

      alloc_substruct(
        p2_entry,
        PageTableFlags::WRITABLE | PageTableFlags::PRESENT,
        true,
      );
    }
  }

  let p1_frame = p2[0].frame().unwrap();
  let p1 =
    unsafe { (p1_frame.start_address().as_u64() as *mut PageTable).as_mut() }
      .unwrap();

  let vcache_state = VCacheState::new();
  let mut vcache = VCache {
    p1: slice::from_mut(p1),
    state: vcache_state,
  };

  // FIXME: In the following we are making two assumptions about VCache at this state
  // 1- Pages returned will be sequential
  // 2- Will only need to access the first PT
  // Both of these assumption are not part of the specification of the VCache
  // meaning, if the implementation changes, this might break!
  for i in 0..P2PAGE_COUNT {
    let p1_virtaddr = match vcache.map(
      PhysFrame::from_start_address(
        p1_frame.start_address()
          + (i * 512 * mem::size_of::<PageTableEntry>()) as u64,
      )
      .unwrap(),
    ) {
      Err(()) => panic!("Could not map VCache P1 table"),
      Ok(page) => page.start_address(),
    };

    // This check has to do with the FIXME above
    if p1_virtaddr != START_ADDR + (PAGE_SIZE * i) as u64 {
      panic!(
        "Time to fix this ugly relianceo n undefined VCache map() behavior"
      );
    }
  }

  let p1_virtadr = START_ADDR;

  let p1_ptr = p1_virtadr.as_u64() as *mut PageTable;
  let p1 = unsafe { slice::from_raw_parts_mut(p1_ptr, P2PAGE_COUNT) };
  // Now let's make a completely valid VCache
  let vcache = VCache::renew(vcache.state, p1);

  // Now we move this to be the global vcache
  VCACHE.call_once(|| RwLock::new(vcache));
}

pub static VCACHE: Once<RwLock<VCache>> = Once::new();

pub struct VCache<'a> {
  p1: &'a mut [PageTable],
  state: Box<VCacheState, EarlyAllocator>, // Using a box, otherwise this beast eats too much stack
}
struct VCacheState {
  p2_freecount: [u16; P2PAGE_COUNT],
  p2_lazycount: [u16; P2PAGE_COUNT],
  p1_age: [[u16; 512]; P2PAGE_COUNT],
}

impl VCacheState {
  fn new() -> Box<Self, EarlyAllocator> {
    Box::new_in(
      Self {
        p2_freecount: [512; P2PAGE_COUNT],
        p2_lazycount: [0; P2PAGE_COUNT],
        p1_age: [[0; 512]; P2PAGE_COUNT],
      },
      EarlyAllocator,
    )
  }
}

impl<'a> VCache<'a> {
  fn renew(
    state: Box<VCacheState, EarlyAllocator>,
    p1: &'a mut [PageTable],
  ) -> Self {
    Self { p1, state }
  }
  pub fn map(&mut self, physframe: PhysFrame) -> Result<Page, ()> {
    // First pass, look for lazy pages already pointing at this physical address
    debug_set_pixel(110, 116, (255, 0, 0).into());
    // for p2i in 0..P2PAGE_COUNT {
    //   let lazy_count = &mut self.state.p2_lazycount[p2i];

    //   if *lazy_count == 0 {
    //     continue;
    //   }

    //   debug_set_pixel(111, 116, (0, 255, 0).into());
    //   for p1i in 0..512 {
    //     let p1_entry = &self.p1[p2i][p1i];
    //     let age = &mut self.state.p1_age[p2i][p1i];
    //     debug_set_pixel(112, 116, (255, 0, 0).into());
    //     let frame = match p1_entry.frame() {
    //       Ok(frame) => frame,
    //       Err(_) => {
    //         debug_set_pixel(112, 116, (0, 0, 0).into());
    //         continue;
    //       }
    //     };
    //     debug_set_pixel(112, 116, (0, 0, 0).into());
    //     if !p1_entry.is_unused() && *age != 0 && frame == physframe {
    //       *age = 0;
    //       *lazy_count -= 1;
    //       let page = Page::<Size4KiB>::from_start_address(
    //         START_ADDR + (PAGE_SIZE * (p2i * 512 + p1i)) as u64,
    //       )
    //       .unwrap();
    //       debug_set_pixel(111, 116, (0, 0, 0).into());
    //       debug_set_pixel(110, 116, (0, 0, 0).into());
    //       return Ok(page);
    //     }
    //   }
    //   debug_set_pixel(111, 116, (0, 0, 0).into());
    // }
    debug_set_pixel(110, 116, (0, 0, 0).into());
    debug_set_pixel(110, 117, (0, 255, 0).into());
    // No good lazy page was found, look for a P2 table with at least 1 free page

    let mut lazy_pages_count = None;

    let mut p2index: Option<usize> = None;
    let mut p1index: Option<usize> = None;

    for i in 0..P2PAGE_COUNT {
      if lazy_pages_count.is_none() {
        lazy_pages_count = Some(self.state.p2_lazycount[i]);
      }
      if self.state.p2_freecount[i] != 0 {
        p2index = Some(i);
        break;
      }
    }
    debug_set_pixel(110, 117, (0, 0, 0).into());
    debug_set_pixel(110, 118, (0, 0, 255).into());
    if p2index.is_none() {
      if lazy_pages_count.is_none() {
        // No free pages were found, and no lazy pages can be freed either
        debug_set_pixel(110, 118, (0, 0, 0).into());
        return Err(());
      }

      // No free pages were found, but some lazy pages exist, allocation can still be done
      for p2i in 0..P2PAGE_COUNT {
        if self.state.p2_lazycount[p2i] != 0 {
          let mut lazy_found_count = 0u16;
          p2index = Some(p2i);
          // Iterate through all P1 tables of this P2 entry until we found all the lazy
          // pages and marked them as free
          debug_set_pixel(111, 118, (0, 255, 0).into());
          for p1i in 0..512 {
            if lazy_found_count >= lazy_pages_count.unwrap_or(0) {
              break;
            }

            let age = &mut self.state.p1_age[p2i][p1i];
            if *age != 0 {
              lazy_found_count += 1;
              *age = 0;
              let mut flags = self.p1[p2i][p1i].flags();
              flags.remove(PageTableFlags::PRESENT);
              self.p1[p2i][p1i].set_flags(flags);

              p1index = Some(p1i);
              // Don't bother invalidating the pages, they will be invalidated when remapped
            }
          }
          debug_set_pixel(111, 118, (0, 0, 0).into());

          self.state.p2_freecount[p2i] = lazy_found_count;
          self.state.p2_lazycount[p2i] = 0;

          // We only clear 1 PDE, do not bother with the other ones as we don't need them now
          break;
        }
      }
    }
    debug_set_pixel(110, 118, (0, 0, 0).into());

    let p2index = p2index.unwrap();
    self.state.p2_freecount[p2index] -= 1;

    // If we went through lazy page invalidation, we already have a target_p1e
    if p1index.is_none() {
      for p1i in 0..512 {
        let flags = self.p1[p2index][p1i].flags();
        if !flags.contains(PageTableFlags::PRESENT) {
          p1index = Some(p1i);
          break;
        }
      }

      // Mark the page as not lazy
      self.state.p1_age[p2index][p1index.unwrap()] = 0;
    }

    let p1index = p1index.unwrap();

    // Now we setup the P1 entry
    let target_p1e = &mut self.p1[p2index][p1index];
    *target_p1e = PageTableEntry::new();
    target_p1e.set_frame(
      physframe,
      PageTableFlags::WRITABLE
        | PageTableFlags::GLOBAL
        | PageTableFlags::PRESENT,
    );

    let virt_addr = START_ADDR + (PAGE_SIZE * (p2index * 512 + p1index)) as u64;
    let page = Page::<Size4KiB>::from_start_address(virt_addr).unwrap();
    MapperFlush::new(page).flush();
    Ok(page)
  }
  pub fn map_many<const N: usize>(
    &mut self,
    phy_addrs: &[PhysFrame; N],
  ) -> Result<[Page; N], ()> {
    let mut result = [None; N];
    for (index, phyaddr) in phy_addrs.iter().enumerate() {
      debug_set_pixel(110, 115, (255, 255, 0).into());
      let page = match self.map(*phyaddr) {
        Ok(page) => page,
        Err(_) => {
          // Unmap previously mapped pages and return error
          for page in result.into_iter().flatten() {
            self.unmap(page);
          }
          debug_set_pixel(110, 115, (0, 0, 0).into());
          return Err(());
        }
      };
      debug_set_pixel(110, 115, (0, 0, 0).into());
      result[index] = Some(page);
    }

    Ok(result.map(|maybe| maybe.unwrap()))
  }
  pub fn remap(
    &mut self,
    page: Page<Size4KiB>,
    physframe: PhysFrame,
  ) -> Result<(), ()> {
    let start_addr = page.start_address();
    if start_addr < START_ADDR || start_addr >= START_ADDR + SIZE as u64 {
      return Err(());
    }

    let target_p2i = Self::p2_index(page);

    let p1_entry = &mut self.p1[target_p2i][start_addr.p1_index()];

    // Already mapped, afandi 3awez y3mel iih from calling this function?
    if !p1_entry.is_unused()
      && p1_entry.frame().is_ok_and(|frame| frame == physframe)
    {
      return Ok(());
    }

    *p1_entry = PageTableEntry::new();

    p1_entry.set_frame(
      physframe,
      PageTableFlags::WRITABLE
        | PageTableFlags::GLOBAL
        | PageTableFlags::PRESENT,
    );
    MapperFlush::new(page).flush();

    Ok(())
  }
  pub fn unmap(&mut self, page: Page<Size4KiB>) {
    let target_p2i = Self::p2_index(page);
    let target_p1i = page.p1_index();

    self.state.p1_age[target_p2i][usize::from(target_p1i)] = 1;
    self.state.p2_lazycount[target_p2i] += 1;
  }
  pub fn unmap_many(&mut self, pages: &[Page<Size4KiB>]) {
    for page in pages {
      self.unmap(*page);
    }
  }

  fn p2_index(page: Page<Size4KiB>) -> usize {
    let start_p2i = START_ADDR.p2_index();
    usize::from(page.p2_index()) - usize::from(start_p2i)
  }
}
