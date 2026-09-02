#include "mod.h"
#include "../memory/method.h"

#define SMALL_ALLOCATION_SIZE 64
#define MEDIUM_ALLOCATION_SIZE 512
#define LARGE_ALLOCATION_SIZE 1024

static void fillMemory(u8 *address, Size size, u8 value);
static void assertMemoryFilled(const u8 *address, Size size, u8 value);
static Boolean rangesOverlap(PhysicalAddress leftAddress, Size leftSize,
                             PhysicalAddress rightAddress, Size rightSize);

/// @brief 测试伙伴分配器的分配隔离、释放复用与块合并能力。
void testBuddyAllocator() {
    PrintWithColor(GREEN, ">>>>>>>>>>> Test BuddyAllocator Start <<<<<<<<<<<\n");

    PhysicalAddress smallAddress = Malloc(SMALL_ALLOCATION_SIZE);
    PhysicalAddress mediumAddress = Malloc(MEDIUM_ALLOCATION_SIZE);
    PhysicalAddress largeAddress = Malloc(LARGE_ALLOCATION_SIZE);

    Assert(smallAddress != NULL);
    Assert(mediumAddress != NULL);
    Assert(largeAddress != NULL);
    Assert(!rangesOverlap(smallAddress, SMALL_ALLOCATION_SIZE,
                          mediumAddress, MEDIUM_ALLOCATION_SIZE));
    Assert(!rangesOverlap(smallAddress, SMALL_ALLOCATION_SIZE,
                          largeAddress, LARGE_ALLOCATION_SIZE));
    Assert(!rangesOverlap(mediumAddress, MEDIUM_ALLOCATION_SIZE,
                          largeAddress, LARGE_ALLOCATION_SIZE));

    fillMemory((u8 *)smallAddress, SMALL_ALLOCATION_SIZE, 0x11);
    fillMemory((u8 *)mediumAddress, MEDIUM_ALLOCATION_SIZE, 0x22);
    fillMemory((u8 *)largeAddress, LARGE_ALLOCATION_SIZE, 0x33);
    assertMemoryFilled((u8 *)smallAddress, SMALL_ALLOCATION_SIZE, 0x11);
    assertMemoryFilled((u8 *)mediumAddress, MEDIUM_ALLOCATION_SIZE, 0x22);
    assertMemoryFilled((u8 *)largeAddress, LARGE_ALLOCATION_SIZE, 0x33);

    // 释放中间块后，以相近大小再次申请，应该复用同一块空间。
    Free(mediumAddress);
    PhysicalAddress reusedAddress = Malloc(400);
    Assert(reusedAddress == mediumAddress);
    fillMemory((u8 *)reusedAddress, 400, 0x44);
    assertMemoryFilled((u8 *)smallAddress, SMALL_ALLOCATION_SIZE, 0x11);
    assertMemoryFilled((u8 *)largeAddress, LARGE_ALLOCATION_SIZE, 0x33);

    Free(smallAddress);
    Free(reusedAddress);
    Free(largeAddress);

    // 所有块释放后应能合并为完整 Buddy 区间。
    PhysicalAddress wholeAddress = Malloc(BUDDY_PAGES * PageSize);
    Assert(wholeAddress == smallAddress);
    Free(wholeAddress);

    PrintWithColor(GREEN, ">>>>>>>>>>> Test BuddyAllocator PASSED <<<<<<<<<<\n");
}

static void fillMemory(u8 *address, Size size, u8 value) {
    for (Size index = 0; index < size; index++) {
        address[index] = value;
    }
}

static void assertMemoryFilled(const u8 *address, Size size, u8 value) {
    for (Size index = 0; index < size; index++) {
        Assert(address[index] == value);
    }
}

static Boolean rangesOverlap(PhysicalAddress leftAddress, Size leftSize,
                             PhysicalAddress rightAddress, Size rightSize) {
    return leftAddress < rightAddress + rightSize
        && rightAddress < leftAddress + leftSize;
}
