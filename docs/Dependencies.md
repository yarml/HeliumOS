Before being able to build and emulate HeliumOS, some dependencies need to be
met.

# Automatic dependencies installation
All the dependencies can be installed with a `sudo make dep`.

This is currently supported on:
* Arch Linux, and its derivatives
* Manjaro (uses `pamac` instead of `pacman`)

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

# Dependencies for running Qemu
- qemu (yeah no shit)
- edk2-ovmf

# Dependencies for debugging
- Qemu's dependencies
- gdb
- tmux
- socat

# Dependencies of the buildsystem
- python3
- GNU Make
- Git
- Basic Unix utilities (Check relevant section)

# Basic Unix utilities
The following programs (or command line equivalent must be present, if their
name is different, their names can be changed in [buildsys/00-vars.mk])
- mkfifo
- umount
- mkdir
- mount
- bash
- touch
- echo
- exit
- socat
- false
- curl
- find
- gzip
- read
- test
- tmux
- true
- cat
- cut
- tar
- sed
- cd
- cp
- ls
- mv
- rm
- tr
- wc

Keep in mind that package names differ from distributions, so you might need to
do more googling.

[buildsys/00-vars.mk]: ../buildsys/00-vars.mk
