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
  - [Debug `debug://`](#debug-debug)
  - [Temporary `tmp://`](#temporary-tmp)

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

## Debug `debug://`
`debug://` is a filesystem which contains some files that reading from/writing
to performs some debugging operation. Most notably, the files `debug://stdout`
and `debug://stderr` write to the debug port (0xE9).

## Temporary `tmp://`
`tmp://` is analog to Unix's `/tmp`, by default it stores files in memory, but
can cache them to disk. On each new start of the system, `tmp://` is clear and
doesn't contain any data nor take any space other than the minimal space a
filesystem takes.

[Bootboot]: https://gitlab.com/bztsrc/bootboot

[sysroots/initrd]: ../sysroots/initrd
