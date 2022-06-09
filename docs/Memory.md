# Physical memory manager
## Features
* Allocate regions from physical address space, with a size hint
* Free regions of the physical address space previously allocated
* Prefer higher addresses when no constraints are forced
* Constraints:
  * Continuous region(Treat size as a must, not a hint)
  * Maximum address(The region should be below a certain address)

## Concepts
### Memory region
A continuous range of addresses that point to valid RAM addresses

### PMM header
The physical memory manager uses a header to keep track of different regions.
The header is an array of variable sized elements(size is always a multiple of 8 bytes), each element is referred to as a segment header.

### Segment header
The segment header consists of a small subheader containing a pointer to the memory area managed by that segment, and the size of the segment,
followed by a bitmap where each bit represents the availability state of a MEM_PS@mem.h sized page.

## Implementation
### Initialization
* Store bootboot's memory map backwards in a buffer(To make the algorithm prefer higher addresses)
* Find a segment in physical space large enough to hold the PMM header(that buffer + bitmaps(The bitmaps are arrays of u64))
* Subtract the size of the PMM header from the size of the segment
* Initialize the bitmaps to 0s(except leftover pages, to 1s)
* Copy the PMM header to its new place
    
### Allocation
* Traverse the memory map found in PMM header and find a segment that is below the maximum address if requested
* Travers that segment's bitmap and find a free page that is still below the maximum address if requested
* Count how many free pages are after this one until the requested size or a used page is reached
* If the request was for a continuous region and size was reached, or if the request wasnt for a continuous region
  * Mark the pages as used and return a mem_pallocation structure containing:
    * The offset of the segment's header after the PMM header
    * The physical address marking the start of the allocated region
    * the actual size of the allocated region(Could be different than the requested size, even if continuous pages are requested)
* Otherwise, find next free page and loop again
* If no region meeting the contraints are found, return a mem_pallocation with all fields set to 0.

### Deallocation
* Use the offset to figure out the segment header address relative to the PMM header
* Mark the requested pages as free

## Interface
* struct mem_pallocation { header_off, padr, size }
* func mem_ppaloc(pheader, size, continuous : bool, below : ptr) : mem_pallocation
* func mem_ppfree(pheader, alloc : mem_pallocation) : void

# Virtual memory manager
yes