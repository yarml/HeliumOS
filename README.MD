```
██   ██ ███████ ██      ██ ██    ██ ███    ███  ██████  ███████
██   ██ ██      ██      ██ ██    ██ ████  ████ ██    ██ ██
███████ █████   ██      ██ ██    ██ ██ ████ ██ ██    ██ ███████
██   ██ ██      ██      ██ ██    ██ ██  ██  ██ ██    ██      ██
██   ██ ███████ ███████ ██  ██████  ██      ██  ██████  ███████
```

# HeliumOS
HeliumOS, a hobby x86_64 operating system whose main goals are learning and testing random ideas from other operating systems.

![HeliumOS Screenshot](/docs/screenshot.png)

*Picture taken before Rust rewrite, the OS does not look like this yet again.*

# Index
- [HeliumOS](#heliumos)
- [Index](#index)
- [Roadmap](#roadmap)
- [Getting HeliumOS](#getting-heliumos)
- [Building HeliumOS](#building-heliumos)
- [Documentation](#documentation)
- [3rd Party tools](#3rd-party-tools)
- [Resources](#resources)

# Roadmap
* [X] Physical memory management
* [X] Virtual memory management
* [X] Interrupts
* [X] Kernel heap
* [X] APIC
* [ ] User space processes
  * [X] System calls basics
  * [X] Scheduling
  * [ ] Build System
* [ ] Basic device drivers
  * [ ] Framebuffer
  * [ ] Keyboard
  * [ ] Mouse
* [ ] System Call API
* [ ] Filesystems
* [ ] Disk
* [ ] Power Management

# Getting HeliumOS
As HeliumOS is hosted in github you can clone the sources with `git` using:

`git clone https://github.com/yarml/HeliumOS.git`.

Otherwise you can download the sources in a [zip file](https://github.com/yarml/HeliumOS/archive/refs/heads/master.zip).

# Building HeliumOS
The build system is completely containerized, meaning as long as you have Docker installed on your system, you will not need to manage further dependencies.

To build the kernel, run `docker compose run build`. Note however, that in the first run, it will download and compile `binutils` which can be resource heavy for some users. The container for the build machine is (currently) 2.17Gib.

The output of the above command are the files in the `out/` directory, the most interesting of which is `out/helium.img`, which can be burned on a USB stick, or tossed into a VM.

To ease the debugging of the development, you can run a Qemu in container alongside gdb, and see all debug output of the operating system
with `docker compose run debug`. Recommendation: Do this on a large enough terminal window.

# Documentation
Documentation for the different sub-components of HeliumOS can be found in [docs/](docs/), albeit it is not always up to date.

# 3rd Party tools
While the code for HeliumOS is completely original, transforming this source
code into binaries that can boot on bare metal is a task which is handled by a
number of 3rd party tools developped indepedently from HeliumOS:

* [Bootboot]: Boot HeliumOS from UEFI environment.
* [Qemu]: Emulate an x86-64 PC.
* [Edk2]: Provide binaries for UEFI for Qemu.
* [GNU Binutils]: ELF file manipulation programs.
* [Rust]: Programming Language.
* [mkbootimg]: Make bootable images from the initrd and kernel binary.

# Resources
This project is only possible because of the following resources:

* [OSDev Wiki]
* [Intel® 64 and IA-32 Architectures Software Developer’s Manuals]
* [AMD64 Architecture Programmer's Manual]
* [Bootboot Protocol Specification]
* [Oracle's Linker and Libraries Guide]
* [ELF Specification]
* [ELF x86-64-ABI psABI]

<!-- "rd Party tools -->
[Bootboot]: https://gitlab.com/bztsrc/bootboot
[GNU Binutils]: https://www.gnu.org/software/binutils/
[Rust]: https://www.rust-lang.org/
[mkbootimg]: https://gitlab.com/bztsrc/bootboot/-/tree/master/mkbootimg
[Qemu]: https://www.qemu.org/
[Edk2]: https://github.com/tianocore/edk2

<!-- Resources -->
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
