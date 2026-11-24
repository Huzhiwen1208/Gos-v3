#ifndef GOS_COMMON_FILE_OP_H
#define GOS_COMMON_FILE_OP_H

/* Shared by GShell, syscall wrappers and GFS. */
enum FileOperationFlags {
    FILE_RECURSIVE = 1U << 0,
    FILE_FORCE = 1U << 1,
    FILE_INTERACTIVE = 1U << 2,
    FILE_NO_CLOBBER = 1U << 3,
    FILE_VERBOSE = 1U << 4,
    FILE_DIRECTORY = 1U << 5,
    FILE_NO_TARGET_DIR = 1U << 6,
    FILE_TARGET_DIRECTORY = 1U << 7
};

#endif
