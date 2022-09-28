# Index
- [Index](#index)
- [Features](#features)
- [Do no forget](#do-no-forget)
- [Optimizations](#optimizations)
- [Cosmetics](#cosmetics)
- [Todo for the far far future](#todo-for-the-far-far-future)

# Features
* [X] Physical memory management
* [ ] Virtual memory management
* [ ] Docker based build system
* [ ] Kernel heap
* [ ] Interrupt handling
* [ ] Load kernel modules
* [ ] IO
* [ ] Handle system calls
* [ ] User space process execution

# Do no forget
* Make page invalidation inter processor proof(When multi processing is supported).

# Optimizations
* Reuse memory taken by bootboot.

# Cosmetics
* Turn all for loops into while loops.

# Todo for the far far future
* Implement HeliumBootboot, instead of relying on the reference implementation.
The implementation can be minimal to Helium's needs at first, that is boot from UEFI.
