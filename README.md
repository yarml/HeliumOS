# HeliumOs
## Index
* [Introduction](#introduction)
* [Getting the sources](#getting-the-sources)
* [Source directory structure](#source-directory-structure)
* [Build system](#build-system)

## Introduction
What is HeliumOs you may ask, it is a hobby operating system designed from 
the ground up(not even based on unix), that is mostly just a learning 
experience, but with some hopes that it will become
someday like your average usable operating system.

Curently, the only platform I am targeting is x86_64, and I have little to no
knowledge about other plateforms to be able to port it, so for now that's
definitely the only platform that will be supported.

## Getting the sources
As HeliumOs is hosted in github you can clone the sources with `git` using:

`git clone https://github.com/YavaCoco/HeliumOS.git`.

Otherwise you can download the sources in a [zip file].


## Source directory structure
* ### [buildsys/]
  * `*.mk` a number of make files that, together, form the build system.
  * `sedscripts/` contains some sed scripts that are used by some targets of
 the build system.

* ### [kernel/]
  * `src/`,`include/`,`stdinc/` conatins the source code for HeliumOs.
  * `*` configurations for the linker, bootboot and mkbootimg.

* ### [sysroots/]
  * `build/` (Build system generated) The prefix where the toolchain to build HeliumOs will be installed
  if built using the build system.
  * `host/` This folder represents the disk which will be passed to the
  virtual macine.
  * `initrd/` This folder represents the file tree for the initrd 
  loaded with bootboot.

* ### [archive/]
  * `*` files waiting to be integrated with the documentation.

* ### ext/ (Build system generated)
  * `src/` contains source code for gcc, binutils, bootboot and mkbootimg.
  * `build` conatins configured build folder for gcc and binutils.

* ### build/ (Build system generated)
  * `*` temporary files used by the build system.

## Build system
The sources come with a build system written in [make](Check [buildsys/]) 
to automate all the steps necessary for the build.

Building HeliumOs is as simple as following 2 steps:
* Installing dependencies, either by executing `sudo make dep`, or if your 
distrubution is not supported(that is anything other than Arch), 
then check [archive/dependencies.txt].
* Making the boot image with `make bootimg`(This will download the source of
and compile binutils, gcc, bootboot and mkbootimg before building the kernel,
which can take a good half hour(or even more depending on your system))

You can then run the boot image from a vm using `make run-qemu`

Or install it in your system with `sudo make install`
(You may want to check [buildsys/70-install.mk] before running this one)

[buildsys/]: buildsys/
[kernel/]: kernel/
[sysroots/]: sysroots/
[archive/]: archive/
[archive/dependencies.txt]: archive/dependencies.txt
[buildsys/70-install.mk]: buildsys/70-install.mk

[make]: https://en.wikipedia.org/wiki/Make_(software)
[zip file]: https://github.com/YavaCoco/HeliumOS/archive/refs/heads/master.zip