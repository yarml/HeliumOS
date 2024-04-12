Start: Jan 8, 2024. Actually, even before, since ~September 2023.
End: Jan 9, 2024.
# State of Affairs
When setting qemu to run any numbers of cores other than 1, the system crashes
at different points each run. Sometimes it crashes after setting up memory,
sometimes when trying to initialize the filesystem. Sometimes it even crashes
when the BSP is already done with its work.

# Debugging session
When stepping with one core at a time, with only 2 cores, the crash happens in
the second core when returning from `proc_ignition_wait_done`. Upon further
investigation, the return address changes before and after blocking core 1.
This suggests that core 1 in its operation writes in core 2's stack. This would
explain why the crash happens when initializing the filsystem, because core 1
allocates a huge chunk of stack memory uncheked. However, this does not explain
why it would sometimes crash while core 1 is still initializing memory, because
as far as I know, core 1 never allocates a large chunk of stack memory at that
point. One possible explanation is that the system always crashes when core 1
is initializing the filesystem, however, the logs don't have time to be received
before qemu resets the system.

Apparetnly, if we leave core 2 blocked in `proc_ignition_wait_done`'s prologue,
the return address is first set to `FFFFFFFFFFE0E808`. If we leave core 2
blocked there, while simultaneously stepping core 1 after mem_init, we find that
core 2's stack has changed, and the return address becomes `FFFFFFFFFFFFFCB8`.
If we still leave core 2 blocked in the prologue and we advance core 1 further
after `__init_stdlib`, the return address changes to `6` :).

The execution of `printd`, `int_init`, & `int_load_and_enable` seems to not
touch core 2's stack, or at least it doesn't reach he return address.

That however does not continue long since `fs_init` now changes the return
address to `FFFF8080000030C8`, followed by `kmod_loadall` which sets it to `0`,
then `__init_stdio` to `7EE160`, then finally, `kmain` which leaves the return
address to its final value of `FFFFFFFFFC15F500`.

Now I have 3 questions:
  - How can I be so stupid to have this many stack overflows?
  - In functions other than `fs_init` & `kmod_loadall`, where do I allocate huge arrays?
  - How does a single core work when it needs more stack memory than it has mapped?

For the second question, I am suspecting it may have to do with things other than
hge arrays, maybe I have a recursive function, altbeit, I don't remember having one.
Actuallythinking about it, isn't mem_vmap recursive?

I have an idea, maybe when I am mapping huge chunks of memory, mem_vmap keeps
calling itself recursively until it fills the stack.

However, I have no clue how to answer the 3rd question. If my code needs more stack that what it has
then it would make sense if it works when other cores exist but are blocked. However, if other
cores don't exist, then bootboot wouldn't map their stack area, leaving any access to the region below the
stack of core 1 to lead to a page fault. But why doesn't it fault when running qemu with only 1 core?

As to the first question, Hi, I am Youssef.

Now let's try to think about ways to fix this. The first obvious thing that needs to be done is
fix `kmain`, `__init_stdio`, & `__init_stdlib`. In principle, these 3 functions do not need
more than 1K stack memory. I have no clue what horrible code my past self wrote to make them
need more than 1K stack memory. As for `fs_init` & `kmod_loadall`, I think I will make them use the heap
because the data they need to load can be arbitrarily large.

Now, other than fixing the problem, we need a way to prevent this from happening in the future, or at
least have an easier means of debugging it. This damn bug put me off developping the OS for 4 months.
I think x86 provides a means of setting a limit to RSP, so that if RSP drops below a certain value it
will trap the CPU, I should set this up and have it handeled correctly.

Another thing I should do is give more stack to each CPU, IIRC, bootboot limits the stack at 1K/core,
so we will need to allocate some stack space for each CPU, and set them up transparently such that their
stack data prior to the switch is copied to the new stack.

I looked into kmain, apparently, the stack overflow happens from `term_setfg`.

Update, setting the stack size by bootboot to 16K fixes the problem. I tought the
limit was 1K. Honestly thinking about it, 1K isn't much, maybe I am just lazy to
figure out what is causing the stack allocations. What's 1K anyway? that's only
128 uint64_t, including return addresses, rbp, and all the other stuff functions need.
I will keep the stack at 16K, it seems reasonable.

As for `fs_init`, it turns out it does not use a lot of stack memory, it directly parses the initrd
loaded by bootboot wherever it was. Whereas `kmod_loadall` already uses heap storage for teh heavy allocations
it needs.

As a way to make debugging these issues easier in the future, I enabled stack protector in GCC to
all functions. This required implementing __stack_chk_fail(), that infamous function I always saw in
Ghidra when reversing an executable. However, when I wanted to test if the stack smashing detector
would work, the problem simply didn't want to happen again. I have no clue why, even after I reversed
the stack size to 1K.

After further testing, I got the `__stack_chk_fail` function to be called, but the print in their causes it to crash.
I think we should have a special stack for __stack_chk_fail, this way even if the stack is so corrupted it is at the boundary
of valid memory space, __stack_chk_fail can still work with a margin.

However it is worthy to note that I couldn't get __stack_chk_fail to get called naturally, I had to step in with a debugger
and do god's work. If I let the OS run naturally with a stack size of 1K, I get some mem_ppalloc panic "Corrupted physical memory header".
No clue why, I tried investigating it, but I am not sure. I will for now work on setting up a stack for __stack_chk_fail.

After making __stack_chk_fail have its own stack, I got the "Stack smashing detected" printed in the logs. However, I am worried
of what will happen if two or more processors have a stack smash, then they both use the same stack.

One solution would be to preallocate stacks for every core as soon as we can, then the small assembly code that loads the stack would
select the correct dynamically preallocated stack. And if stack smashing occurs before this step, we would just hope that it doesn't happen
to 2 cores at once.

Another imo better solution would be to implement a lock. Once a core stack smashes, honestly we don't care what happens as long as
we get an eligible error message, other cores who happen to stack smash at the same time don't need to report it if the first one did.
We can just have a lock for the singular stack. I will go implement this now.

Done. Now there is only the mystery of the corrupted physical memory header.