# Index
- [Index](#index)
- [Features](#features)
  - [Roadmap](#roadmap)
  - [Back burner](#back-burner)
  - [I should do these now, but I'm lazy](#i-should-do-these-now-but-im-lazy)
- [Do no forget](#do-no-forget)
- [Optimizations](#optimizations)

# Features
## Roadmap
* [X] Physical memory management
* [X] Virtual memory management
* [X] Kernel heap
* [X] Interrupts
* [ ] Filesystems
* [ ] initrd filesystem
* [ ] Kernel modules
* [ ] Basic device drivers (Keyboard, Mouse, etc)
* [ ] Power Management
* [ ] System calls
* [ ] User space processes
* [ ] Multiprocessing

## Back burner
* [ ] Docker based build system
* [ ] HeliumBootboot

## I should do these now, but I'm lazy
* Implement mem_vumap()

# Do no forget
* Make page invalidation inter processor proof(When multi processing is
  supported).

# Optimizations
* Reuse memory taken by bootboot.
