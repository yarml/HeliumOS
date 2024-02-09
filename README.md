```
██   ██ ███████ ██      ██ ██    ██ ███    ███  ██████  ███████
██   ██ ██      ██      ██ ██    ██ ████  ████ ██    ██ ██
███████ █████   ██      ██ ██    ██ ██ ████ ██ ██    ██ ███████
██   ██ ██      ██      ██ ██    ██ ██  ██  ██ ██    ██      ██
██   ██ ███████ ███████ ██  ██████  ██      ██  ██████  ███████
```

# HeliumOS
HeliumOS, a hobby operating system I am working on my free time. The long term
goal of the OS is to have a mixture of all the nice features of other operating
systems.

Currently, the only supported platform is x86-64 UEFI, and I have little
to no knowledge about other platforms, so they may never be supported.

If you find issues with compiling the kernel checkout
[Known Issues](#known-issues).

![HeliumOS Screenshot](/docs/screenshot.png)

# Current State of Affairs
This list is a subset of what is found in [Progress](#progress), and covers the
priorities.

- [X] Multiprocessing P1: Make the system not crash when multiple cores are
      running. To move to the next stage I just need to make it so that AP cores
      hang on a loop. Nothing fancy.
- [X] APIC: Handle interrupts, make the OS useful, talk between processors.
- [ ] Processes: Provide a way to run user space processes, can use the initrd
      for now.
- [ ] Multiprocessing P2: Efficiently distribute work between processors.

# Index
- [HeliumOS](#heliumos)
- [Current State of Affairs](#current-state-of-affairs)
- [Index](#index)
- [Getting the sources](#getting-the-sources)
- [Build system](#build-system)
- [Progress](#progress)
- [Documentation](#documentation)
- [Contributions](#contributions)
- [Source directory structure](#source-directory-structure)
  - [buildsys/](#buildsys)
  - [kernel/](#kernel)
  - [tools/](#tools)
  - [sysroots/](#sysroots)
  - [docs/](#docs)
  - [ext/ (Build system generated)](#ext-build-system-generated)
  - [build/ (Build system generated)](#build-build-system-generated)
  - [cache/ (Build system generated)](#cache-build-system-generated)
- [3rd Party tools](#3rd-party-tools)
- [Resources](#resources)
- [Known Issues](#known-issues)

# Getting the sources
As HeliumOS is hosted in github you can clone the sources with `git` using:

`git clone https://github.com/yarml/HeliumOS.git`.

Otherwise you can download the sources in a [zip file].

# Build system
The sources come with a build system written in [make](Check [buildsys/])
to automate all the steps necessary for the build.

Building HeliumOS is as simple as following 3 steps:
* Run just `make` to initialize the build system. It will simply say "Nothing to be done for 'nothing'"
* Installing dependencies, either by executing `sudo make dep`, or if your
distrubution is not supported(that is anything other than Arch & Manjaro),
then check [dependencies].
* Making the boot image with `make bootimg`(This will download the source of
and compile binutils, gcc, bootboot and mkbootimg before building the kernel,
which can take a good half hour(or even more depending on your system))

You can then run the boot image from a vm using `make run-qemu`.

The output file `build/helium.img` can be flashed to a USB and used to boot
on real hardware, although I rarely test that.

Important Note: When you just clone the repository, make sure you run `make`
as a normal user first before installing dependencies. Otherwise, if `make` is
run the first time as root, it will create buildsystem files as root, and they
will be unusable later by a normal user.

# Progress
The following list of features may change order any time depending on
what I want to work on.

* [X] Physical memory management
* [X] Virtual memory management
* [X] Kernel heap
* [X] Interrupts
* [X] Filesystems
  * [X] initrd filesystem
  * [X] Debug filesystem
  * [X] Kernel terminal filesystem
* [X] Kernel modules (Removed)
  * [X] Make kernel terminal a module
* [X] APIC
* [X] Basic device drivers
  * [X] Framebuffer (Screen)
  * [X] Keyboard
  * [ ] Mouse
* [ ] User space processes
* [ ] Multiprocessing
* [ ] System calls
* [ ] Rework filesystem API.
* [ ] Disk
* [ ] Power Management

# Documentation
Current documentation for HeliumOS can be found in [docs/].

Below is a list of some key documentations:
* [Code Style] (Boring)
* [Memory management] (I hate memory management)
* [Filesystem] (My personal favorite)

# Contributions
If by any chance you are interested in this project, you are free to study,
fork, modify, and open a pull request. Any type of work is welcome, be it
bug fixes(there exist many!), new features, or documentation.

# Source directory structure
## [buildsys/]
  * `*.mk` Make files that, together, form the build system.
  * `late/*.mk` Other make files that together form the buildsystem, but are
    included later in the buildsystem.
  * `sedscripts/` sed scripts that are used by some targets of the build system.
  * `pyscripts/` Python scripts used by the build system.
  * `shscripts` Shell scripts used by the buildsystem
  * `IDE` Pre-made configurations for some IDEs

Documentation for the buildsystem can be found here [Buildsystem].

## [kernel/]
`src/`,`include/`,`stdinc/` Source code for HeliumOS.

`link.ld` Linker configuration.

`bootimg.json` mkbootimg configuration.

`bootboot.config` Bootboot configuration.

## [tools/]
Some tools used by the buildsystem to generate other files used by
the buildsystem.

## [sysroots/]
`build/` (Build system generated) The prefix where the toolchain to build
HeliumOS will be installed.

`host/` represents the disk which will be passed to the virtual
macine (unused yet).

`initrd/` The file tree for the initrd loaded with bootboot.

## [docs/]
Documentation for HeliumOS.

## ext/ (Build system generated)
`src/` contains source code for gcc, binutils, bootboot and mkbootimg.

`build` conatins configured build folders for gcc and binutils.

## build/ (Build system generated)
Temporary files used by the build system.

The target file of the entire buildsystem is `build/helium.img`.

## cache/ (Build system generated)
Contains files downloaded from the internet to avoid redownloading them
each build.

# 3rd Party tools
While the code for HeliumOS is completely original, transforming this source
code into binaries that can boot on bare metal is a task which is handled by a
number of 3rd party tools developped indepedently from HeliumOS:

* [Bootboot]: Boot HeliumOS from UEFI environment.
* [GNU Compiler Collections]: Compile C source code.
* [GNU Binutils]: Providing linker and other binary file manipulation programs.
* [Nasm]: Assemble assembly source code.
* [GNU Make]: Execute build commands.
* [GNU efi]: Used by bootboot.
* [mkbootimg]: Make bootable images from the initrd and kernel binary.
* [Qemu]: Emulate an x86-64 PC.
* [Edk2]: Provide binaries for UEFI for Qemu.

# Resources
This project is only possible because of the following resources:

* [OSDev Wiki]
* [Intel® 64 and IA-32 Architectures Software Developer’s Manuals]
* [AMD64 Architecture Programmer's Manual]
* [Bootboot Protocol Specification]
* [Oracle's Linker and Libraries Guide]
* [ELF Specification]
* [ELF x86-64-ABI psABI]

# Known Issues
- In bootboot some `libc` functions require `const char *` parameters,
but `unsigned char *` are passed instead, if build fails there, you might need
to fix them manually (only 4 changes required) (Will be fixed when Helium
Bootboot is implemented)

- Some bootboot file is missing an include of `stdint.h`. If you get an error
about `uint64_t` undefined, just add the include in the file the error is coming
from.

<!-- Raw links  -->
[buildsys/]: buildsys/
[kernel/]: kernel/
[sysroots/]: sysroots/
[docs/]: docs/
[tools/]: tools/
[modules/]: modules/

<!-- Named links  -->
[dependencies]: docs/Dependencies.md
[Code Style]: docs/Code-Style.md
[Memory management]: docs/Memory.md
[Filesystem]: docs/Filesystem.md
[Buildsystem]: docs/Buildsystem.md

<!-- External links  -->
[make]: https://en.wikipedia.org/wiki/Make_(software)
[zip file]: https://github.com/yarml/HeliumOS/archive/refs/heads/master.zip
[OSDev Wiki]: https://wiki.osdev.org/Expanded_Main_Page

[Intel® 64 and IA-32 Architectures Software Developer’s Manuals]:
https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html

[AMD64 Architecture Programmer's Manual]:
https://www.amd.com/en/support/tech-docs/amd64-architecture-programmers-manual-volumes-1-5

[Bootboot Protocol Specification]:
https://gitlab.com/bztsrc/bootboot/-/raw/master/bootboot_spec_1st_ed.pdf

[Oracle's Linker and Libraries Guide]:
https://docs.oracle.com/cd/E23824_01/html/819-0690/toc.html

[ELF Specification]: https://refspecs.linuxfoundation.org/elf/elf.pdf

[ELF x86-64-ABI psABI]:
https://gitlab.com/x86-psABIs/x86-64-ABI/-/jobs/artifacts/master/raw/x86-64-ABI/abi.pdf?job=build

[Bootboot]: https://gitlab.com/bztsrc/bootboot
[GNU Compiler Collections]: https://gcc.gnu.org/
[GNU Binutils]: https://www.gnu.org/software/binutils/
[Nasm]: https://www.nasm.us/
[GNU Make]: https://www.gnu.org/software/make/
[GNU efi]: https://sourceforge.net/projects/gnu-efi/
[mkbootimg]: https://gitlab.com/bztsrc/bootboot/-/tree/master/mkbootimg
[Qemu]: https://www.qemu.org/
[Edk2]: https://github.com/tianocore/edk2
