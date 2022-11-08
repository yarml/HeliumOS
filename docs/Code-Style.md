
# Index
- [Index](#index)
- [Variable naming](#variable-naming)
  - [Suffix `_size`](#suffix-_size)
  - [Suffix `_len`](#suffix-_len)
  - [Suffix `_off`](#suffix-_off)
  - [Suffix `_idx`](#suffix-_idx)
- [Variable typing](#variable-typing)
- [Struct definition](#struct-definition)
- [Include](#include)
  - [Order](#order)
  - [Categories](#categories)
    - [Simple includes](#simple-includes)
    - [Subfolder includes](#subfolder-includes)
    - [Source folder include](#source-folder-include)
  - [Example](#example)
- [Assembly](#assembly)
- [Debug printing](#debug-printing)
  - [Mid-Implementation debug printing](#mid-implementation-debug-printing)
  - [Value printing format](#value-printing-format)

# Variable naming
## Suffix `_size`
Use the suffix `_size` for variables holding the size of a structure in bytes.

## Suffix `_len`
Use the suufix `_len` for variables holding the length of a structure in elements unit.

## Suffix `_off`
Use `_off` for variables holding the offset from a pointer in bytes.

## Suffix `_idx`
Use `_idx` for variables holding the index from a pointer in elements units.

As a general rule(but not always true), with `ESIZE` the size of a single element:

```
%_size = %_len * ESIZE
%_off = %_idx * ESIZE
```

# Variable typing
Variable types should come first, followed with any modifiers, followed by pointer sign, then by any pointer modifiers.
Contrary to the popular `const type`, `type const` is to be used. The pointer sign should be next to the variable name not type name,
for example `type *name` not `type* name`. This rule was changed in the middle of the developement, so `type* name` pointer declaration can stil be found in the source code.

# Struct definition
All struct definitions in the source code must follow the example for a struct called `example_struct`
```c
struct EXAMPLE_STRUCT;
typedef struct EXAMPLE_STRUCT example_struct;
struct EXAMPLE_STRUCT
{
  ...
};
```
Like shown in the example, the actual struct should follow the `EVERYTHING_UPPERCASE` naming convention, while the typedef that is used in the source code is `everything_lowecase`.

# Include
Use `#include <%>` instead of `#include "%"` whenever possible.

## Order
Include statement have to be ordered following their categories first, then from the longest to the shortest include

## Categories
Include statement come in three categories that must be separated by empty lines and ordered in the same order here.

### Simple includes
Those are includes that include files from `kernel/stdinc` or `kerne/include`.

### Subfolder includes
Those are includes that includes files from subfolder of `kernel/stdinc` or `kerne/include`.

### Source folder include
Those are includes that include files from `kernel/src` (i.e includes that are forced to use `#include "%"` format).

## Example
The best example for how to #include is in [mem.c] as it has all three categories.

# Assembly
Assembly code is to always be minimized, and when necessary it should always be abstracted in a function written in an `.asm` file or a preprocessor macro with inline assembly if the instruction is simple enough.

# Debug printing
## Mid-Implementation debug printing
When implementing a new feature, it is often necessary to print out a lot of information until that feature is completely implemented,
figuring out later which printfs should be removed and which ones should stay can be tricky, this is why Helium source provides `tpf`
standing for *temporary printf* so that it could be easily found and removed when no longer needed

## Value printing format
When an internal value of the system is the be displayed out, the format `decription(value1, ...)\n` is to be used.
When only displaying a message, the format `message.\n` is to be used.

[mem.c]: ../kernel/src/mem/mem.c

