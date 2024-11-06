Start: Nov 5, 2024
End: Ongoing

# Project Statement
The OS until now is more of a POC, dirty code everywhere and using libraries whenever possible for tasks as
fundamental as memory allocation.

We should move away from that, a complete rewrite using no external runtime libraries, designed for completeness and efficiency instead
of the get it done asap to see results mindset.

# Requirements
- All parts of the code must be multi-processor safe; currently even physical memory allocation is not thread safe, we just rely on prayers.
- All modules of the OS must be complete; done are the days where writing a memory manager means implementing allocation, and leaving deallocation as a noop with a TODO
- APIs must be final, usable, & good.

# Progress
- [ ] Sync point
- [ ] Logging
- [ ] SysInfo
- [ ] PIC
- [ ] Basic GDT
- [ ] Physical Memory Management
- [ ] Kernel Mappings
- [ ] Hart Ignition
- [ ] ...
- [ ] Interrupts

# Terminology
- Hart: A hardware thread. be it a physical core, logical core, or separate processor. It can be scheduled to do work or left hanging.
- Process: Sum of a memory map and a number of threads.
- Thread: Unit of scheduling.
- Memory Map: self explanatory.
- Kernel Task: A task which periodically runs sometimes before the scheduler gives way for a thread.

# Implmentation Details
## Sync Point
Sync points are a way for all processors to wait for each other until they all reach the same/equivalent code
points

