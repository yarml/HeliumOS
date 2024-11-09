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
- [ ] Sync
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
A fair Read/Write lock.

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
