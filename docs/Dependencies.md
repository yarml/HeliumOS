Before being able to build and emulate HeliumOs, some dependencies need to be met.

# General dependencies
- Basic unix tools(cp, mkdir, ls yata yata yata)
- Make (Not sure if anything other than GNUMake will work)

# Automatic dependencies installation
All the dependencies can be installed with a `sudo make dep`.

This is currently supported on:
* Arch Linux, and its derivatives

# Dependencies for building binutils and gcc
- bison
- flex
- gmp
- mpc
- mpfr
- texinfo
- C/C++ compiler

# Dependencies for building mkbootimg
- zip
- C/C++ compiler

# Dependencies for building bootboot
- gnu-efi
- C/C++ compiler

# Dependencies for running Qemu:
- qemu (yeah no shit)
- edk2-ovmf

# Dependencies for debugging
- Qemu's dependencies
- gdb
- tmux
- socat

# Additional dependencies
Some features offered by the build system require additional features
- python3

Keep in mind that package names differ from distributions, so you might need to do more googling.
