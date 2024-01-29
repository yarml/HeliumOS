Start: Jan 29, 2024.
End: .
# Objective
Have the ability to load a user space program, provide a syste call for printing to kterm
and to exit the program.

# Progress
- [ ] Load flat binary to user space addresses, and be able to call it
- [ ] Switch CPU to user mode
- [ ] Implement print & exit syscalls
- [ ] Load ELF executable
- [ ] Setup build system to make this process streamlined

# Initial steps
First thing I want to do is remove the entire file system, I will replace the initrd
reading with its own system, the filesystem will only be used by user space programs, not
the kernel itself.

