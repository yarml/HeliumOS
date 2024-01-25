Start: Jan 9, 2024.
End: .
# Objective
Implement APIC. I want to have a timer set in every core, have a working keyboard
driver, figure out what APIs I will need to make available so that this driver can be moved
out as a kernel module.

# Initial steps
I looked at interrupts/apic.c, and it turns out I already wrote a huge base for the implementation
of APIC. I already have the structures defined etc. I am ready through intel's System Programming Guide
and I am unsure about a few things, namely, when setting up the APIC, do I need to lock a mutex? All
local APICs use the same memory address for their memory accessed registers, and as far as I read,
the cores will hande reads and wrtes to that region of memory invisibly, movs to and from those
addresses do not make it to the system bus, so I think I don't need a lock, I means I do not necessarily
need one. I feel like I will implement a lock if only at least to have nice log messages anyway.

I have a running question and that is can every core share the same kernel area mappings? Or will I eventually
need to make each core have a separate kernel area mapping? In handsight, I would have first said yes, they should
all share the same kernel mappings, afterall, all cores are running the same kernel. But now that I reached APICs,
I got an idea of a scenario where it would be possible for different kernel mappings to be necessary for each core.

Imagine APICs addresses weren't the same, if I wrote to FEE00000 from core2 I would be configurin core1's APIC, in such
case I would either have to map all APICs to all cores each one at a different virtual address. Or I would have made it
so that each CPU can access the same constant address to configure its own APIC, requiring a different kernel mapping
per core.

Anyways, for now all cores can share the same kernel mapping, and eventually when every core will have its own CR3 to
support different user space processes running in parallel, every core will have its own memory table that just so happens
to share the same kernel mappings with other distinct memory tables of different cores.

Coming back to APIC, I have to designate a place in kernel memory for the APIC registers. I chose the area right
after the initrd.

# Current Problem
Okay, so things have been going pretty smoothly, if I wasn't determined to test this super correctly
first try I would have probably already setup the timer now in all cores, but Qemu is driving me crazy.
CPUID[16H] is supposed to report an approximation of the processor's clock speed, but for some reason
Qemu decided not to implement that, it simply returns 0. I tested the OS on my hardware and it returns
the correct CPU frequence 2600MHz, so the problem isn't in my code. I looked in the Internet, apparently
VMWare doesn't implement CPUID[16H] either, but instead it exposes the processor frequency through CPUID[0x400010]
WHY??? Why now I need to have special code for VMs. Apparently these VMs do something clever, and that is
sometimes when Intel's specification says that a bit returned from CPUID will be set to 0 always, the VMs will set it to
1 so that the guest OS will know it is running in a VM, this is the case of CPUID[1H].ECX bit 31. Qemu sets it to 1 even
though in x86 it is always 0. Very clever. While reading the documentation I thought Intel specifically said that bit will
be set to 1 to indicate a VM, but nope, they just say it is always 0. Another thing is CPUID[0x40000010], and all the CPUIDs
in the range near that are in the documentation marked as invalid CPUIDs that will never be used in the future, but VMs
still use them to report various information. I'm wondering if Intel engineers who wrote that manual are doing it on purpose.
Why would they mark an entire range of CPUIDs not as reserved, but as "will never use". I just read parts of the RISCV manual
and when they did something that was supposed to be hacked by VMs and other stuff they would simply share it directly in those
comment sections "Hey this bit can be used by software implementations of RISCV". While with Intel it's as if they are trying to
hide the fact they are giving space for VMs to express themeselves. Idk, it's just how I felt.

# Update
Okay listen, it has been a few days, the APIC timer simply doesn't want to behave the way it should, I have no clue why.
I will simply now move on to the IO APIC, try making a keyboard driver. Hopefully this one goes smooth.

# Update 2
It has been multiple days since the lats entry. I found out that to find where the IO/APIC base address is I need to parse
the ACPI, so that's what I have been working on. It has been going smooth so far. I am trying to design a system that would
allow me to have a module handle a particular ACPI entry if it reports supporting it.


Let me tell you something funny. Something I find very funny. Something almost makes me want to cry. I emptied kmain(),
it used to print the HeliumOS header, a fake prompt, then it printed PCI devices, I just made it return 0 immediatly, got
rid of the header and prompt, and moved pci_probe to _start, and suddenly, the timer works? I got no clue why, it was probably something
in PCI probe, I will go investigate what that fucker does. It looks so innocent, only a triple for loop. I have no idea why it makes
it not work.

So now, I can mark the first step of the objective done, we have a timer running. Now I can continue working on the keyboard
driver. From what I read so far, there is a chance IOAPIC will also have that emulated PS/2 keyboard like PIC. If it does
it should be in the interrupt source override entry of the MADT, however, neither in my real hardware, nor my emulator, do I get
such entry for IRQ base 1(the PS/2 emuator). From what I read, if I want to know what IRQ a particular PCI device uses, I need to do
some ACPI AML stuff. I am not familar with that at all, and to be quite honest, I am scared of it, it's an OOP language to talk with
devices???? Idk, I want to first look if there is a way I can get the IRQ of a device from the PCI table directly (although I think
if there was such a way I would have found it mentioned before the AML thingy). Another thing is that from what I know ACPI AML
stuff is related to SSDT/DSDT entries. In Qemu I have no such entries. I do have them on real hardware however. Maybe there is an
option I can pass to qemu to make it generate them. Anyways, if I can't find the info I want from PCI table, I will then look at
AML. But for now, I am happy the timer is working.

Bingo `Alternatively, you could just use MSI or MSI-X, and skip complicated ACPI.` I think this means there is a way for PCI
to setup the interrupt directly or something?

Yesterday Night, I spent a lot of time reading about USB, I read about 60% of the USB page on OSDEV and started reading the overview
chapter of Inteæ's xHCI documentation, because for some reason I had assumed that an integrated keyboard emulates a USB keyboard, then
out of curiosity I ran `lsusb` to see how Linux sees my keyboard, but strange, it didn't list my integrated keyboard, is it just a PCI
device? But it doesn't appear on `lspci` either. Where the hell is the integrated keyboard, I thought. I list devices in
`/proc/bus/input/devices` and I do see my keyboard there, it says it is on Bus 11, but what Bus 11? it's not PCI nor USB. I learn about
this command `lshw` which should list all hardware in my PC in a tree format, and I see PCI, and USB is connected to PCI, but the keyboard is
in its own branch of the tree, it just says it's an input device. Then it came unto me, it's a PS/2 keyboard. I thought PS/2 keyboard were
a thing of the past, an intergated keyboard surely isn't just a PS/2 device right? Welp, it looks like it is. Now I put some dirty code
to make the IOAPIC map IRQ1 to interrupt 0xD0, and it fucking works. In OSDev they clearly state that relying on the IOAPIC to have the
same IRQs as legacy PIC is not good, and they recommend looking in the AML to see how PCI devices are using the IRQs of the IOAPIC, but
that would have been a lot of work only for me to eventually discover that my computer does not even have a USB keyboard.

Now I am working on cleaning up the code I used to make the keyboard work, notably I should at the very least respect any potential
redirection entry in the MADT for IRQ1.

After that I think I will make small simple kernel shell, implement an `lspci` and `lsacpi`, and not print them by default.

Then, finally, I can mark this step of the OS as DONE!

Oh right, I also want to make this PS/2 driver a kernel module, which means I need a way to make kernel modules register
interrupt handlers. I will work on that, also, I probably need to work on a dependency system for my kernel modules, the `kshell`
module which will handle the kernel shell should depend on `ps2`, or a more generic `kbd` feature, and maybe make `ps2` provide the `kbd`
feature. For now I will just make sure to use both of them.

Or what about this, what if I make a system for message passing between kernel modules. I am thinking something like `klisten("kbd", buf, len)` in the
kshell part, and `ksend("kbd", buf, len)` in the PS/2 driver part.

Going back to interrupt allocation, I should probably also allocate this interrupt in only one processor, so the logic for interrupt initialization
should change, I can make life simple and make BSP handle all interrupts from external devices, which doesn't look that bad tbh.