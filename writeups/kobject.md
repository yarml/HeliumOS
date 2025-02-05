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
- [X] Sync
  - [X] Barrier
  - [X] Once
  - [X] RwLock
  - [X] Mutex
- [X] Logging
- [ ] SysInfo
- [ ] PIC
- [ ] Basic GDT
- [ ] Physical Memory Management
  - [X] Low Memory Management
  - [ ] Middle Memory Management
  - [ ] High Memory Management
- [ ] Virtual Memory Management
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
## Sync
### Barrier
Barriers are a way for all processors to wait for each other until they all reach the same/equivalent code
points

POC:
```rust
// This gets executed by multiple harts in parallel
fn example() {
  static BARRIER = Barrier::new();

  func1();
  BARRIER.sync();
  func2();
  BARRIER.sync();
}
```

In the above example, no hart will execute func2 until all harts are done with func1.
And no hart will return until all harts are done with func2.

Barriers can be reused (as shown in the example). But care must be taken when that is done, and
preferably only done in true parallel situations.

This is close in similarity to `std::sync::Barrier`. Except it does not sync different threads, but rather physical Harts.

## Once
Once is a synchronization primitive that allows a global variable to be set once thread-safely, then only be read accessible.
It is useful when a global variable cannot be initialized at compile time.

POC:
```rust
static EXAMPLE: Once<usize> = Once::new();
fn example() {
  EXAMPLE.get(); // -> Option::None
  EXAMPLE.init(|| 3);
  EXAMPLE.get(); // -> Option::Some(&3)
  EXAMPLE.init(|| 4); // panic!
}
```

## RwLock
A fair Read/Write lock. Based on FOLL from https://doi.org/10.1145/1583991.1584020.
Currently the FOLL based lock is not implemented, as that would require kernel heap.
They will be needed really only when we have kernel tasks etc. For now a NaiveRwLock
exists to bootstrap.

POC (not perfect but meh):
```rust
static EXAMPLE: RwLock<usize> = RwLock::new(5);

fn hart1() {
  let example = EXAMPLE.read();
  println!("{example}");
  let example = example.upgrade();
  *example = 7;
}
fn hart2() {
  let example = EXAMPLE.read();
  println!("{example}");
}
fn hart3() {
  let example = EXAMPLE.write();
  *example = 4;
  let example = example.downgrade();
  println!("{example}");
}
```
# Mutex
A mutually exclusive lock.

## Logging
The kernel comes with 4 log levels: Debug, Info, Warn, Error.

Logs are written to the framebuffer and 0xE9 IO port (both behind feature flags).

The module `logging` exports 4 macros which are meant to be used to print logs: `debug`, `info`, `warn`, `error`, each for their obvious log level.

A log line contains the HartId from where the log came, the log level, and the log message.

## Physical Memory Management
### Low Memory Management
Low memory is the region of physical memory going in the range [1Mib, 16Mib). Areas
lower than that are never used for any purpose and not managed by the kernel.

Low Memory is only used to allocate memory for drivers wihch require it. General memory requests from
application or kernel management will never fall back to Low Memory, even if that means a memory request
cannot be fullfilled(i.e system is considered out of memory even though all 15Mib of Low Memory are still unused).

Low Memory requests come in 2 sizes only, 64Kib frames on 64Kib boundary, and 128Kib frames on 128Kib boundary.
There is no support for N continuous frames of any size.

Due to its small size, all of Low Memory is managed using a Buddy allocator of 6 64bit words in total.

## Middle Memory Management
WIP
