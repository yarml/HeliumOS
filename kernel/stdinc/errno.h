#ifndef CSTD_ERRNO_H
#define CSTD_ERRNO_H

#define errno (*__get_errno())

int *__get_errno();

#define ENOMEM (1)

#endif