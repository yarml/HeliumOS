Warning: The filesystem API is going to be redesigned in the future.

# Filesystem
A file in HeliumOS denotes any system resource that has a path associated with
it. The most obvious example of this is space on disk devices, but it is not
limited to disks. Like Unix systems, everything is a file in HeliumOS, ranging
from network sockets to device control files and much more. Files are the way
all communication is done between the kernel and user space applications.

# Index
- [Filesystem](#filesystem)
- [Index](#index)
- [Path](#path)
- [File Operations](#file-operations)
- [Filesystem Types](#filesystem-types)
- [Standard Filesystems](#standard-filesystems)
  - [Initrd `initrd://`](#initrd-initrd)
  - [Debug `dbg://`](#debug-dbg)
  - [Temporary `tmp://`](#temporary-tmp)
- [Helium Filesystem Interface](#helium-filesystem-interface)
  - [`fs_open`](#fs_open)
  - [`fs_search`](#fs_search)
  - [`fs_dirof`](#fs_dirof)
  - [`fs_close`](#fs_close)
  - [`fs_read`](#fs_read)
  - [`fs_pull`](#fs_pull)
  - [`fs_skip`](#fs_skip)
  - [`fs_write`](#fs_write)
  - [`fs_append`](#fs_append)
- [LibC](#libc)
  - [`fopen`](#fopen)

# Path
A file path in HeliumOS may look like this `initrd://sys/config`. The part
before `:` is called the filesystem name, in this case `initrd`. The filesystem
name is used to know which part of the kernel should respond to the operation
done on the file. After `://` the path of the file within the filesystem may
mean different things depending on the filsystem. Some basic filesystems that
HeliumOS mounts on boot are `initrd://`, `debug://`, `tmp://` (And more). All
of these filesystems are documented later in this document.

# File Operations
Files may or may not support a number of operations, most commonly:
| Operation   | Abbreviation |
| ---------   | ------------ |
| Read        | R            |
| Write       | W            |
| Pull        | P            |
| Append      | A            |
| Tell Size   | Ts           |
| Resize      | Rs           |
| Remove self | Rm           |

Directories also have a number of common operations:
| Operation    | Abbreviation |
| ------------ | ------------ |
| Add entry    | Ae           |
| Remove entry | Re           |
| List Entries | L            |
| Remove self  | Rm           |

# Filesystem Types
Depending on the operations files and directories in a filesystem support,
a filesystem may fall into a common category

| Category Name | File Operations  | Directory Operations | Example Use case |
| ------------- | ---------------- | -------------------- | ---------------- |
| Normal        | R W A Ts Rs Rm   | Ae Re L Rm           | Disk drives      |
| Immutable     | R Ts             | Le                   | Memory fs        |
| Pipe          | P A              | **                   | Interprocess com |
| Funnel        | A                | **                   | Device control   |
| Source        | P                | **                   | Status query     |

**: Can be anything

Obviously not all filesystems fall into these categories, they may be a mixture
or their own thing, this is just to explain some words used in the source code
of HeliumOS.

# Standard Filesystems
## Initrd `initrd://`
`initrd://` is an immutable in-memory filesystem loaded from disk by [Bootboot].
It is a `tar` file built from the directory [sysroots/initrd] (So if you add
a file `a/test.txt` to that directory, you can see it in HeliumOS as
`initrd://a/test.txt`).

## Debug `dbg://`
`dbg://` is a filesystem which contains some files that reading from/writing
to performs some debugging operation. Most notably, the files `dbg://stdout`
and `dbg://stderr` write to the debug port (0xE9).

## Temporary `tmp://`
`tmp://` is analog to Unix's `/tmp`, by default it stores files in memory, but
can cache them to disk. On each new start of the system, `tmp://` is clear and
doesn't contain any data nor take any space other than the minimal space a
filesystem takes.

# Helium Filesystem Interface
HeliumOS exposes a number of functions to deal with files and directories, below
is a documentation of the most important of those functions.

The interface on its own does not give enough information about what each
operation does, because the underlying filesystem is what gets to decide what
happens for each opeartion

## `fs_open`
Takes as input a string that represents the filesystem name, and an array of
strings each denoting a filesystem node. This function is not meant to be
called directly, and is the back bone to functions like `fs_search` and
`fs_dirof`. It also increments the reference count of the file opened, as such
`fs_close` should be called when the reference is no longer used otherwise
filesystems that can unload nodes from memory will keep the node in memory
even when it is unused.

## `fs_search`
Parses a path into an array of node names, then calles `fs_open` to open the
file/directory.

## `fs_dirof`
Like fs_search, except it stops at the directory before the last node, then
calls `fs_open` to open the directory.

## `fs_close`
Decrements the reference count of a file, enabling some filesystems to
completely remove the file node from memory until it is needed again.

## `fs_read`
Reads the specified amount of bytes(or less) from the file given as parameter at
a certain offset. The filesystem at the end gets to define what any of those
words mean.

*Needs capability FSCAP_FREAD*

## `fs_pull`
Pull data from a file. The filesystem gets to define what pulling actually
means.

*Needs capability FSCAP_FPULL*

## `fs_skip`
Skips the specified amount of bytes from the file, the filesystem decides what
that means. The default implementation provided by HeliumOS if the filesystem
does not define skip is calling `fs_pull` on a dummy buffer of the specified
amount of bytes.

*Needs capability FSCAP_FPULL*

## `fs_write`
Writes the specified amount of bytes(or less) to the file given as parameter at
a certain offset. The filesystem at the end gets to define what any of those
words mean.

*Needs capability FSCAP_FWRITE*

## `fs_append`
I could have also called it `fs_push` if it didn't start with a `p` like `pull`.
Pushes a specified amount of bytes to a file. The filesystem gets the define
what pushing actually means.

*Needs capability FSCAP_FAPPEND*

# LibC
HeliumOS implements an interface that is very close(but not compatible with)
the libc FILE interface. Whether I should keep that interface is a topic I am
considering. But as of now, HeliumOS has functions `fopen`, `fclose`, `fread`,
`fwrite`, etc in addition to some non standard functions such as `fpull` and
`fappend`. The behavior of all these functions is different in a way or another
than that of standard libc, as they are closer in their working to Helium's file
interface.

## `fopen`
The `fopen` that is included with HeliumOS differs from the standard `fopen` in
the `mode` argument. Traditional modes like `r+`, `wb` and others are not
supported. Instead `mode` is a string of capabilities that the file to open
should support.

Below is a list of modes supported by `fopen`:
| Notation | Name   | HeliumOS File Capability |
| -------- | ------ | ------------------------ |
| r        | Read   | FSCAP_FREAD              |
| w        | Write  | FSCAP_FWRITE             |
| p        | Pull   | FSCAP_FPULL              |
| a        | Append | FSCAP_FAPPEND            |

As a result, to open a file for both reading and writing, instead of using
`r+` or `w+` like in standard libc, one would use `rw` in HeliumOS kernel.

It is worthy to note that some modes may shadow other modes in some operations,
for example, if a file was both open in writing and appending mode, then
functions like `fprintf` and `fputs` will prefer writing instead of appending.

The functions `fwrite`, `fappend`, `fread`, `fpull` always do what their names
suggest or error if the file was not opened with the appropriate mode.

[Bootboot]: https://gitlab.com/bztsrc/bootboot

[sysroots/initrd]: ../sysroots/initrd
