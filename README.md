# HeliumOS
HeliumOs is a hobby operating system that started as a learning experience(and is still just that), the plan is to take the cool concepts from other operating systems while keeping the os very light(that's where the name Helium came from)

Right now the OS is written only for x86 architectures, but ports to x86_64 are planned

# How to compile?
The source files come with a makefile at the top directory, a simple make command(preferrably from the top directory) is enough to compile the kernel and generate an iso with grub as bootloader

# Run
You can grab the iso generated and use it in any virtual machine, or burn it in a USB stick or something

The makefile provided provides both `make bochs` and `make qemu` for quick testing in those two VMs

# All the make commands
`make iso` or `make`: Builds the kernel image and creates an iso with grub as boot loader

`make kernel`: Builds the kernel image, no iso is generated

`make clean`: Clean all the build files

`make bochs`: Generates the iso if not already done then run it using bochs as VM

`make qemu`: Generates the iso if not already done then run it using qemu as VM

`make qemu-gdb`: Same as `make qemu`, but sets up Qemu to listen for gdb

`make gdb`: Launches gdb to work with Qemu
