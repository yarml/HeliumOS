# Overview of the build system
The HeliumOS project uses [Make] to automate the build process, this is because Make(as opposed to other build systems) lets you control every command executed for the build, which is essential for something like compiling a kernel

Make has also the upside of being super customizable

# Getting the sources
The HeliumOS project is currently only hosted in Github, so that's the only place where you can get the source code.
Assuming you have `git` installed in your system, you can get the sources with the following command:

`git clone https://github.com/YavaCoco/HeliumOS.git`

If you don't have git installed(then you should get it), you can download the sources from [here][Download]

# The Makefile version
As this documentation may be lagging behind the actual existing building functionality, it is (semi) important that you check the current makefile version by running any make command(or `make nothing` if you don't want to do any action), and compare it to this documentation's version(now it is 0.2), if they are different, then either the sources you have are old, or this documentation wasn't updated yet, which means that some functions described here may not work or work differently. This shouldn't be something to worry about though, as many functionalities that may change are probably secondary, or can be easily worked around.

# Dependencies
Before you can go forward and start configuring the kernel and the build, you must first make sure that the following dependencies are available(preferably the latest version)

For the sake of clarity, I'll divide the dependencies into three categories: **Essential dependencies**, **architecture specific dependencies** and **optional dependencies**
## Essential dependencies
This category contains dependencies that cannot be substituted with other tools(unless the other tool was designed to copy the original ones, but you get my point):
* GNU Make
* GNU m4
* Basic unix commands(cp, rm, mkdir)

If you have other tools that can completely replace, then you may specify them in [make/build_config/10_tools.mk]

## Target architecture specific dependencies
Depending on which architecture and toolchain you use, you'll need a different set of assemblers, compilers, and linkers.
### The i686 Architecture
By default, the i686 architecture proposes two choices for toolchains:
#### LLVM-NASM
The LLVM-NASM toolchain requires:
* A Netwide assembler (NASM by default)
* Clang (C++20) (Should support output to i686-elf)
* LLVM

There is a great chance that the clang version provided in your repos already support output to i686-elf. If it isn't the case, then you may consider building LLVM and Clang from source yourself.
#### GNU-NASM
The GNU-NASM toolchain requires:
* A Netwide assembler (NASM by default)
* GCC/G++ (C++20) (Should support i686-elf output)
* binutils (Should support i686-elf output)

There is little to no chance that a GNU toolchain for the i686-elf architecture is available in your repos, so you will probably need to build one yourself from source.

### Define your toolchain
Alternatively, you can define your own toolchain, with your own set of assemblers, compilers, and linkers, have a look [here][Toolchains] if you want to know how to do so, you may also consider contributing your custom toolchain to the kernel if you wish. When making a custom toolchain though, you should make sure it can generate output for the desired architecture(If that wasn't obvious).
## Optional dependencies
The build system also supports directly running the OS(as described in [Running the kernel]), but to do that you'll need one of the two VMs:
* Bochs (only support x86 architectures)
* Qemu
# Configuring the kernel
The HeliumOS was designed with customization in mind, as a result many aspects of the kernel can be modified easily without needing other parts of the code to change.

Configurations are stored  in 2 main folders:
### Core kernel configuration
[kernel/core/config/config]
### Architecture specific configuration
[kernel/arch/<arch>/config/config]

The configuration files are either C or C++ header files, and they should be documented with each variable and its usage.

Note: If changing any of those variables causes the kernel to break(at build time or run time), then please report it as a bug, as those variables are supposed to be configurable.

# Configuring the build
Once you have configured the kernel to your taste, it is time to configure the build itself. Depending on your system, this step might be necessary, but fear not, it isn't a complicated process.

Nearly all the configuration variables can be found in [make/build_config], each file there is documented with each variable and its usage(unless it is clear from the name), changing those variables shouldn't(in theory) cause any problems to the build process(Please report it as a bug if it does)

Toolchains are the only exception that cannot be configured from those files, instead, toolchains are found in `kernel/arch/<Target architecture>/make/toolchains/`, you can take a look at [kernel/arch/demo/make/toolchains/EXAMPLE.mk] if you want more details about what each variable defined by the toolchain is used for.

Some variables can be directly set in the make command line(`make VAR1=VALUE VAR2=VALUE <target>`), they are summed up in the following table:


|Variable|Description|Possible values|
|-|-|-|
|ARCH|Sets the target architecture|Valid architectures can be found in kernel/arch|
|TOOLCHAIN|Sets the toolchain to use|Architecture dependent|
|CC|C compiler (overrides the one defined by the toolchain)|Any valid C compiler that expects the output file as the first argument|
|CXX|C++ compiler (overrides the one defined by the toolchain)|Any valid C++ compiler that expects the output file as the first argument|
|AS|Assembler (overrides the one defined by the toolchain)|Any valid assembler that expects the output file as the first argument|
|LD|Linker (overrides the one defined by the toolchain)|Any valid linker that expects the output file as the first argument|
|SL|Static linker or archiver (overrides the one defined by the toolchain)|Any valid static linker that expects the output file as the first argument|
|QEMU|Qemu virtual machine|Any valid qemu command|
|BOCHS|Bochs virtual machine|Any valid bochs command|
|CP|
|MKDIR|
|RM|
|M4|
|BDIR|Build directory|
|RDIR|Run directory|

# Building the kernel
Once everything is well configured to your needs, you can build the kernel, this can be done easily by executing `make`, this will generate an iso at `$(ISORUN)`(Configurable in [make/build_config/00_paths.mk]), which defaults to `run/helium-<Target architecture>`.iso

Individual components of the kernel can be made separately if that is what you want(as opposed to building the entire kernel and packaging it in an iso), the following make commands will only generate a specific part of the kernel:
* `make arch`: Generates a static library (.a) containing architecture specific object files, outputs the file to `$(ARCHOUT)` which defaults to `build/arch-<Target architecture>.a`
* `make kernel`: Generates the kernel elf file without putting it in an iso, the output file is found at `$(KOUT)`, which defaults to `build/helium-<Target architecture>.kernel`

You can also compile/assemble an individual file `X` with `make build/obj/X.o`(where `X` is the path to the file from the top directory), so to compile `kernel.cpp`, the command would be: `make build/obj/kernel/core/src/init/kernel.cpp.o` assuming `kernel.cpp` is found at `kernel/core/src/init/kernel.cpp`

You can preprocess a [template file][Templates] with path `X` relative to the top directory with the command `make X`, for example, to generate the file `isodir/boot/grub/grub.cfg`, you would use `make isodir/boot/grub/grub.cfg`. Template files that get preprocessed are found in `templates/X.m4`, for file `X`

# Known Issues
## \`mcopy\` invocation failed
There is a chance when you are making the iso with `make iso`(or executing a target that implicitly generates the iso like `bochs` or `qemu`), that the following or a similar error pops up: `grub-mkrescue: error: 'mcopy' invocation failed`
That is a bug in the 4.0.33 version of mtools, as for now the only solution is to downgrade mtools to 4.0.29, or if you are reading this in the future upgrading to a newer version may solve the issue

[Make]: https://en.wikipedia.org/wiki/Make_(software)
[Download]: https://github.com/YavaCoco/HeliumOS/archive/refs/heads/master.zip

[make/build_config/10_tools.mk]: ../make/build_config/10_tools.mk
[kernel/core/config/config]: ../kernel/core/config/config
[kernel/arch/<arch>/config/config]: ../kernel/arch/demo/config/config
[make/build_config]: ../make/build_config
[kernel/arch/demo/make/toolchains/EXAMPLE.mk]: ../kernel/arch/demo/make/toolchains/EXAMPLE.mk
[make/build_config/00_paths.mk]: ../make/build_config/00_paths.mk

[Templates]: TODO:LinkIsNotValidYet
[Running the kernel]: TODO:LinkIsNotValidYet
[Toolchains]: TODO:LinkIsNotValidYet
