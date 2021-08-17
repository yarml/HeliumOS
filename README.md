# Warning 
Currently there is 0 code in this branch, things are being refactored

In other words, don't download this version of the code if you want to compile the kernel or something

Take a version one commit behind

# HeliumOS
HeliumOs is a hobby operating system that started as a learning experience(and is still just that), the plan is to take the cool concepts from other operating systems while keeping the os very light(that's where the name Helium came from)

Right now the OS is written only for x86 architectures, but ports to x86_64 are planned

# Getting the sources
HeliumOS is currently only hosted in Github
Provided that `git` is installed in your system, you can get the latest sources with:

`git clone https://github.com/YavaCoco/HeliumOS.git`

# How to build the kernel?
The source files come with a makefile at the top directory, a simple `make` command is enough to build the kernel and generate an iso with grub as bootloader

The make build system can be heavily configured, if you want to know more about it take a look at the [documentation for the building process](docs/Building.md)

# Run
You can grab the iso generated and use it in any virtual machine, or burn it in a USB stick or something

The makefile provides both `make bochs` and `make qemu` for quick testing in those two VMs
