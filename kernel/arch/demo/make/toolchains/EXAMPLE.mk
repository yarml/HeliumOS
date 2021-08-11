# An architecture should define at least one toolchain to use to build the sources
# A toolchain includes an assembler, C/C++ compiler, linker and static linker

# Below is the list of variables defined in this type of file
AS       ?= nasm -o               # AS is the variable used for the assembler
CC       ?= gcc  -o               # CC is the variable used for the C compiler
CXX      ?= g++  -o               # CXX is the variable used to identify the C++ compiler
LD       ?= g++  -o               # LD is the variable used to indentify the linker
SL       ?= ar   -o               # SL is the variable used to identify the static linker
ASFLAGS  :=                       # Flags for assembler
CFLAGS   :=                       # Flags for C compiler
CXXFLAGS :=                       # Flags for C++ compiler
CCFLAGS  :=                       # Flags for both C and C++
LDFLAGS  := -Lbuild/ -larch-x86.a # Flags for linker
SLFLAGS  :=                       # Flags for static linker
# Note that for the linker flags, the make system expects the flags to include the arch
# specific code as a static library
# You may notice the -o at the end of each command, that is because the makefile expects
# the first argument to be the output file, so if you want to change them, you should make
# sure the first argument is the output file
