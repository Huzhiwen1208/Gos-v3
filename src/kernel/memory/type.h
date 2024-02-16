#pragma once

#include "../common/type.h"

#define MAX_PAGE_COUNT 8192
#define PageSize 4096
#define PageSizeBits 12

typedef struct FrameAllocator {
    /* Pages[i]: 0b00000xxx;
        x0: 1 means free, 0 means used
        x1: 0 means kernel, 1 means user
        x2: 0 means not dirty, 1 means dirty
    */
    u8 Pages[MAX_PAGE_COUNT];
    u32 TotalFreePageCount;
    u32 KernelFreePageCount;
} FrameAllocator;

typedef struct AddressRangeDescriptor {
    u64 BaseAddress;
    u64 Length;
    u32 Type;
} AddressRangeDescriptor;

// Buddy

#define BUDDY_BLOCKS 1024
#define BUDDY_PAGES 16

typedef struct BuddyBlock {
    PhysicalAddress BaseAddress;
    Size BlockSize;
    Boolean IsUsed;
    i32 Next;
} BuddyBlock;

typedef struct BuddyAllocator {
    BuddyBlock Blocks[BUDDY_BLOCKS];
    MachineMode Mode;
} BuddyAllocator;