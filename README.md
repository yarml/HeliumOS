# HeliumOS
HeliumOS, a hobby operating system I am working on to keep myself from
overthinking life. The long term goal of the OS is to have a mixture of
all the nice features of other operating systems.

Currently, the only supported platform is x86-64 UEFI, and I have little
to no knowledge about other platforms, so they may never be supported.

# Index
- [HeliumOS](#heliumos)
- [Index](#index)
- [Getting the sources](#getting-the-sources)
- [Build system](#build-system)
- [Progress](#progress)
- [Documentation](#documentation)
- [Contributions](#contributions)
- [Source directory structure](#source-directory-structure)
- [3rd Party tools](#3rd-party-tools)
- [Resources](#resources)

# Getting the sources
As HeliumOS is hosted in github you can clone the sources with `git` using:

`git clone https://github.com/yarml/HeliumOS.git`.

Otherwise you can download the sources in a [zip file].

# Build system
The sources come with a build system written in [make](Check [buildsys/])
to automate all the steps necessary for the build.

Building HeliumOS is as simple as following 2 steps:
* Installing dependencies, either by executing `sudo make dep`, or if your
distrubution is not supported(that is anything other than Arch & Manjaro),
then check [dependencies].
* Making the boot image with `make bootimg`(This will download the source of
and compile binutils, gcc, bootboot and mkbootimg before building the kernel,
which can take a good half hour(or even more depending on your system))

You can then run the boot image from a vm using `make run-qemu`, or install
it in your system with `sudo make install`
(You should want to check [buildsys/70-install.mk] before running this one)

# Progress
The following list of features may change order any time depending on
what I want to work on.

* [X] Physical memory management
* [X] Virtual memory management
* [X] Kernel heap
* [X] Interrupts
* [ ] Filesystems
* [ ] initrd filesystem
* [ ] Kernel modules
* [ ] Basic device drivers (Keyboard, Mouse, etc)
* [ ] Power Management
* [ ] System calls
* [ ] User space processes
* [ ] Multiprocessing
* [ ] Disk

# Documentation
Current documentation for HeliumOS can be found in [docs/].

Below is a list of some key documentations:
* [Code Style]
* [Memory management]

# Contributions
If by any chance you are interested in the project you are free to fork, modify
and make a pull request to merge. Be it bug fixing, or new features, everything
is welcome (But nobody will anyway, I feel like I'm talking to myself, this
is embarassing, I'm so lonely 🫤. Also it's my birthday 🥳).

# Source directory structure
* ## [buildsys/]
  * `*.mk` a number of make files that, together, form the build system.
  * `sedscripts/` contains some sed scripts that are used by some targets of
 the build system.
  * `pyscripts/` contains some Python scripts that do statistics on source code
  * `shscripts` contains shell scripts used by the buildsystem
  * `IDE` contains pre-made configurations for some IDEs

* ## [kernel/]
  * `src/`,`include/`,`stdinc/` contains the source code for HeliumOS.
  * `link.ld` configurations for the linker.
  * `bootimg.json` configurations for mkbootimg.
  * `bootboot.config` Bootboot configuration.

* ## [sysroots/]
  * `build/` (Build system generated) The prefix where the toolchain to build
    HeliumOS will be installed if built using the build system.
  * `host/` This folder represents the disk which will be passed to the
  virtual macine.
  * `initrd/` This folder represents the file tree for the initrd
  loaded with bootboot.

* ## [docs/]
  * `*` Documentation for HeliumOS.

* ## ext/ (Build system generated)
  * `src/` contains source code for gcc, binutils, bootboot and mkbootimg.
  * `build` conatins configured build folders for gcc and binutils.

* ## build/ (Build system generated)
  * `*` temporary files used by the build system.

# 3rd Party tools
While the code for HeliumOS is completely original, transforming this source
code into binaries that can boot on bare metal is a task which is handled by a
number of 3rd party tools developped indepedently from HeliumOS:

* [Bootboot]: Boot HeliumOS from UEFI environment.
* [Gnu Compiler Collections]: Compile C source code.
* [Nasm]: Assemble assembly source code.
* [Gnu Make]: Execute build commands.
* [Gnu efi]: Used by bootboot.
* [mkbootimg]: Make bootiable images from the initrd and kernel binary.
* [Qemu]: Emulate an x86-64 PC.
* [Edk2]: Provide binaries for UEFI for Qemu.

# Resources
This project is only possible because of the following resources:

* [OSDev Wiki]: Compresses the information found in the below manuals to be more
  intelligible, although sometimes innacurate.
* [Intel® 64 and IA-32 Architectures Software Developer’s Manuals]: Especially
  the 3rd volume.
* [AMD64 Architecture Programmer's Manual]: Especially the 2nd volume.
* [Bootboot Protocol Specification]: Describing how Bootboot works.

<!-- Raw links  -->
[buildsys/]: buildsys/
[kernel/]: kernel/
[sysroots/]: sysroots/
[docs/]: docs/
[buildsys/70-install.mk]: buildsys/70-install.mk

<!-- Named links  -->
[dependencies]: docs/Dependencies.md
[Code Style]: docs/Code-Style.md
[Memory management]: docs/Memory.md

<!-- External links  -->
[make]: https://en.wikipedia.org/wiki/Make_(software)
[zip file]: https://github.com/yarml/HeliumOS/archive/refs/heads/master.zip
[OSDev Wiki]: https://wiki.osdev.org/Expanded_Main_Page
[Intel® 64 and IA-32 Architectures Software Developer’s Manuals]: https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html
[AMD64 Architecture Programmer's Manual]: https://www.amd.com/en/support/tech-docs/amd64-architecture-programmers-manual-volumes-1-5
[Bootboot Protocol Specification]: https://gitlab.com/bztsrc/bootboot/-/raw/master/bootboot_spec_1st_ed.pdf
[Bootboot]: https://gitlab.com/bztsrc/bootboot
[Gnu Compiler Collections]: https://gcc.gnu.org/
[Nasm]: https://www.nasm.us/
[Gnu Make]: https://www.gnu.org/software/make/
[Gnu efi]: https://sourceforge.net/projects/gnu-efi/
[mkbootimg]: https://gitlab.com/bztsrc/bootboot/-/tree/master/mkbootimg
[Qemu]: https://www.qemu.org/
[Edk2]: https://github.com/tianocore/edk2
