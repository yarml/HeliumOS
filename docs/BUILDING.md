# Overview of the build system
The HeliumOS project uses [Make](https://www.gnu.org/software/make/) to automate the build process, this is because Make(as opposed to other build systems) lets you control every command executed for the build, which is essential for something like compiling a kernel

Make has also the upside of being super customizable

# Getting the sources
The HeliumOS project is currently only hosted in Github, so that's the only place where you can get the source code.
Assuming you have `git` installed in your system, you can get the sources with the following command:

`git clone https://github.com/YavaCoco/HeliumOS.git`

If you don't have git installed(then you should get it), you can download the sources from [here](https://github.com/YavaCoco/HeliumOS.git)

# The Makefile version
As this documentation may be lagging behind the actual existing building functionality, it is (semi) important that you check the current makefile version by running `make makefile-version`, and compare it to this documentation's version(now it is 0.2), if they are different, then either the sources you have are old, or this documentation wasn't updated yet, which means that some functions described here may not work or work differently. This shouldn't be something to worry about though, as many functionalities that may change are probably secondary, or can be easily worked around.

# The build system structure
Nearly all the make files are stored in make/
# Work In Progress
