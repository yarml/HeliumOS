# Compilers, assemblers and linkers
# They should expect the first argument to be the output file path
# Thus -o are added
# If you change those make sure you add the equivalent of -o parameter
AS       := nasm -o
CC       := i686-elf-gcc -c -o
CXX      := i686-elf-g++ -c -o
LD       := i686-elf-g++ -o

# Basic unix commands
MKDIR := mkdir -p
RM    := rm -rf
CP    := cp
# A command to make sure that an iso is multiboot v1 compliant
ISMULTIBOOT := grub-file --is-x86-multiboot
# A command to generate a multiboot compliant iso, make sure the first argument is the output file
MKISO := grub-mkrescue -o

# Bochs and Qemu VMs
BOCHS := bochs
QEMU := qemu-system-x86_64
