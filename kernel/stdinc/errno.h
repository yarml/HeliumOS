#ifndef CSTD_ERRNO_H
#define CSTD_ERRNO_H

#define errno (*__get_errno())

int *__get_errno();

// Note; Some of these errors are not used by HeliumOS in their POSIX
// meaning. While for the standard C library I try to use the same meanings
// that they have in POSIX, for Helium specific functions, they can mean
// all sort of additional things
#define ENOMEM (1)
#define EINVAL (2)
#define ENOENT (3)
#define EOPNOTSUPP (4)
#define ENAMETOOLONG (5)
#define ENOFS (6) // Not POSIX
#define ENOTDIR (7)

#endif