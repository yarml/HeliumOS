
# Index
- [Index](#index)
- [Variable naming](#variable-naming)
  - [Suffix `_size`](#suffix-_size)
  - [Suffix `_len`](#suffix-_len)
  - [Suffix `_off`](#suffix-_off)
  - [Suffix `_idx`](#suffix-_idx)
- [Include](#include)
  - [Order](#order)
  - [Categories](#categories)
    - [Simple includes](#simple-includes)
    - [Subfolder includes](#subfolder-includes)
    - [Source folder include](#source-folder-include)
  - [Example](#example)
- [Assembly](#assembly)

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


[mem.c]: ../kernel/src/mem/mem.c

