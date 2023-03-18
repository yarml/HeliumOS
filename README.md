# HeliumOs
What is HeliumOs you may ask, it is a hobby operating system designed from
the ground up(not even based on unix), that is mostly just a learning
experience, but with some hopes that it will become
someday like your average usable operating system.

Curently, the only platform I am targeting is x86_64, and I have little to no
knowledge about other plateforms to be able to port it, so for now that's
definitely the only platform that will be supported.

# Index
- [HeliumOs](#heliumos)
- [Index](#index)
- [Getting the sources](#getting-the-sources)
- [Source directory structure](#source-directory-structure)
- [Build system](#build-system)
- [Documentation](#documentation)

# Getting the sources
As HeliumOs is hosted in github you can clone the sources with `git` using:

`git clone https://github.com/yarml/HeliumOS.git`.

Otherwise you can download the sources in a [zip file].


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
    HeliumOs will be installed if built using the build system.
  * `host/` This folder represents the disk which will be passed to the
  virtual macine.
  * `initrd/` This folder represents the file tree for the initrd
  loaded with bootboot.

* ## [docs/]
  * `*` Documentation for HeliumOs.

* ## ext/ (Build system generated)
  * `src/` contains source code for gcc, binutils, bootboot and mkbootimg.
  * `build` conatins configured build folders for gcc and binutils.

* ## build/ (Build system generated)
  * `*` temporary files used by the build system.

# Build system
The sources come with a build system written in [make](Check [buildsys/])
to automate all the steps necessary for the build.

Building HeliumOs is as simple as following 2 steps:
* Installing dependencies, either by executing `sudo make dep`, or if your
distrubution is not supported(that is anything other than Arch & Manjaro),
then check [dependencies].
* Making the boot image with `make bootimg`(This will download the source of
and compile binutils, gcc, bootboot and mkbootimg before building the kernel,
which can take a good half hour(or even more depending on your system))

You can then run the boot image from a vm using `make run-qemu`, or install
it in your system with `sudo make install`
(You may want to check [buildsys/70-install.mk] before running this one)

# Documentation
Current documentation for HeliumOs can be found in [docs/].

Below is a list of some key documentations:
* [Code Style]
* [Memory management]
* [Mathematical proof for formulas used in the OS]

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
[Mathematical proof for formulas used in the OS]: docs/Math.md

<!-- External links  -->
[make]: https://en.wikipedia.org/wiki/Make_(software)
[zip file]: https://github.com/YavaCoco/HeliumOS/archive/refs/heads/master.zip
