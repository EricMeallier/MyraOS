#ifndef LIBC_USER_TYPES_H
#define LIBC_USER_TYPES_H

#include <stdint.h>
#include <stddef.h>

// Process and user identifiers
typedef int     pid_t;
typedef int     uid_t;
typedef int     gid_t;

// Device and inode identifiers
typedef uint32_t dev_t;
typedef uint32_t ino_t;
typedef uint32_t mode_t;
typedef uint32_t nlink_t;

// File and offset types
typedef int     fd_t;
typedef int64_t off_t;
typedef int64_t ssize_t;

// Time-related
typedef int64_t time_t;
typedef int64_t suseconds_t;
typedef int64_t clock_t;

// Signal number
typedef int     sig_t;

// Page size
typedef uint32_t page_t;

// Permissions
typedef uint32_t perm_t;

#endif // LIBC_USER_TYPES_H
