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

//Stable

#define StableBlockSize 512 //固定分区大小
#define StablePages 6 //固定分区数量
#define StartAddress (1024ULL * 1024ULL + 64ULL * 1024ULL) //起始地址为 1MB+64KB
typedef struct StableBlock {
    PhysicalAddress BaseAddress; //物理地址
    Size BlockSize; //分区大小
    Boolean IsUsed; //是否被使用
    i32 Next; //下一个固定分区索引
} StableBlock;

typedef struct StableAllocator {
    StableBlock Blocks[StablePages]; //存储固定分区块信息
} StableAllocator;