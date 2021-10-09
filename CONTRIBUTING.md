# Work In Progress
too lazy to do this now :/

mmm Maybe if you want to contribute you can start by writing this =]

The following is just things that I would like to include in this file

# Code style
This section is not completely written yet right now, so the best way to know the code style used is to look at the existing code

Variables should have self explanatory names, even if long

Minimize the use of macros, constexpr are better for C++ code

Although files written in C are supported by the build system, it is better if we stick to C++. `extern "C"` can be used to call assembly functions from C++

Complicated expressions should always be accompanied with a comment explaining what they do in layman's terms. For example:

```C++
for(; mem_map_len < mbt_info->mmap_length;
    ++mem_map_size, mem_map_len +=
        reinterpret_cast<multiboot::mmap_entry*>
        (mbt_info->mmap_addr + mem_map_len)->size 
        + sizeof(
            reinterpret_cast<multiboot::mmap_entry*>
            (mbt_info->mmap_addr + mem_map_len)->size
        )/* since the size variable itself is not counted */)
/* in layman's terms: for each element in the memory map */
{
    // Do sorcery here
}
```
*Example taken from [kernel/arch/i686/memory/init.cpp]:detect_memory()*
# Assembly
Assembly code should preferably have a pseudo code explaining what the code does, this pseudo code should obviously be written as a comment

Each assembly files should end with a section where this pseudo code is written. This is because assembly code is generally hard to understand alone

## Pseudo code syntax
The pseudo code used should respect some rules(although breaking some if necessary won't cause too much problem), this is to make sure it can be understood without ambiguity

compile time variables can be referenced with $, so for example the variable in NASM declared like: `FOO equ 10`, can be referenced with `$FOO`/`${FOO}`/`$(FOO)`/anything that is obvious in the pseudo code section, 

Pseudo code can also include english sentences if they are not very ambiguous

Pseudo code can also use C++ types, or custom ones if they are obvious(such as `u32` ot `i32`), sticking to some convention for literal types is probably a good idea, so you should probably use the most used types names

It is also possible to use some standard-like attributes with functions/variables (like `[[noreturn]]`) as long as their meaning is clear.

Registers can be referenced with %(so `%eax` for eax...)

TODO: We should make types and attributes more standard and complete


[kernel/arch/i686/memory/init.cpp]: kernel/arch/i686/memory/init.cpp
