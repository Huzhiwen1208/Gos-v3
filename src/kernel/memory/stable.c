#include "mod.h"
#include "../lib/mod.h"

StableAllocator globalStableAllocator; // 定义全局分配变量

static i32 getFreeBlockIndex();                // 获取空闲的固定分区块的索引
static void freeBlock(i32 index);              // 释放固定分区块
static StableBlock *getStableBlock(i32 index); // 根据索引获取相应的分区块的指针

PhysicalAddress StableMalloc(Size size) {
    i32 pre = 0;
    i32 p = getStableBlock(0)->Next;
    while (p != -1) {
        if (getStableBlock(p)->IsUsed == FALSE && getStableBlock(p)->BlockSize >= size) {
            break;
        }
        pre = p;
        p = getStableBlock(p)->Next;
    }
    // 从第一个分区块开始遍历，直到找到一个未被使用且大小满足要求的分区块。在遍历过程中，用变量 pre 记录上一个分区块的索引，用变量 p 记录当前分区块的索引。

    if (p == -1)
        Panic("No enough memory when malloc: size=%d", size); // 未找到空闲分区，输出错误消息并终止程序执行
    getStableBlock(p)->IsUsed = TRUE;
    return getStableBlock(p)->BaseAddress; // 找到了满足条件的分区块，将其标记为已使用，并返回其起始地址
}

void StableFree(PhysicalAddress address) {
    i32 pre = 0;
    i32 p = getStableBlock(0)->Next; // 从第一块遍历，直到找到具有指定物理地址的分区块。
    while (p != -1) {
        if (getStableBlock(p)->BaseAddress == address) {
            break;
        }
        pre = p;
        p = getStableBlock(p)->Next;
    } // 如果找到了具有指定物理地址的分区块，则跳出循环，p 指向的分区块即为要释放的分区块
    getStableBlock(p)->IsUsed = FALSE; // 标为未使用
}

void globalStableAllocatorInit() {
    for (Size i = 0; i < StablePages; i++) {
        globalStableAllocator.Blocks[i].BaseAddress = StartAddress + i * StableBlockSize; // 起始地址加上该分区块在稳定分配器中的索引乘以分区块大小，确保每个分区块的起始地址都不同。
        globalStableAllocator.Blocks[i].BlockSize = StableBlockSize;                      // 固定的分区块大小
        globalStableAllocator.Blocks[i].IsUsed = FALSE;                                   // 分区块未被使用
        globalStableAllocator.Blocks[i].Next = i + 1;
    } // 初始化分配器，遍历每个分区

    // 计算最后一个分区块的基地址和大小
    PhysicalAddress baseAddress = (PhysicalAddress)273 * PageSize; // 页号*页大小
    Size size = StablePages * StableBlockSize;

    globalStableAllocator.Blocks[StablePages - 1].Next = -1; // 将最后一个分区块的 Next 设置为 -1，表示链表的末尾
}

static i32 getFreeBlockIndex() {
    for (i32 i = 0; i < StablePages; i++) {
        if (globalStableAllocator.Blocks[i].IsUsed == FALSE) {
            return i;
        }
    }

    Panic("No free block when get free block index");
}

// 遍历空闲分区块，找到一个未被使用的分区块，并返回其索引。如果没有找到未被使用的分区块，触发一个错误。
static StableBlock *getStableBlock(i32 index) {
    return &globalStableAllocator.Blocks[index]; //& 操作符取得了该分区块的地址
} // 根据索引获取固定分区块的指针，并返回该指针。