Start: Jan 29, 2024.
End: .
# Objective
Have the ability to load a user space program, provide a syste call for printing to kterm
and to exit the program.

# Progress
- [X] Load flat binary to user space addresses, and be able to call it
- [ ] Switch CPU to user mode
- [ ] Implement print & exit syscalls
- [ ] Load ELF executable
- [ ] Setup build system to make this process streamlined

# Initial steps
First thing I want to do is remove the entire file system, I will replace the initrd
reading with its own system, the filesystem will only be used by user space programs, not
the kernel itself.

Now that is done, I will make a dummy flat binary executable that just contains some mov
instructions, call it in kernel mode, and mark the first step as done.

And that is done, the only "difficulty" I had is that I was trying to map a virtual address
to another virtual address, this stupid thing cost me an hour.

What I have now is just a binary that moves 42 into rax then returns, and in effect it's like
an int function that returns 42, and I can see its result. I would really love to make it print something
but calls use relative addresses, and the 2G where I load the flat binary is very far from where kterm_print would be
and I don't want to spend time making trampolines, so I will just make it print after I switch to user mode
and implement system calls.
For now, I can move to the next step, switching to usermode, this probably would make the OS crash since ret won't be enough
to return to the kernel.