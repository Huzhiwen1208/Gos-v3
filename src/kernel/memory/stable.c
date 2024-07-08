#include "mod.h"
#include "../lib/mod.h"

StableAllocator globalStableAllocator; // 定义全局分配变量

static i32 getFreeBlockIndex();                // 获取空闲的固定分区块的索引
static void freeBlock(i32 index);              // 释放固定分区块
static StableBlock *getStableBlock(i32 index); // 根据索引获取相应的分区块的指针

// TODO: 补全StableMalloc、StableFree、globalStableAllocatorInit函数

PhysicalAddress StableMalloc(Size size) {
    Panic("StableMalloc is not implemented");
}

void StableFree(PhysicalAddress address) {
    Panic("StableFree is not implemented");
}

void globalStableAllocatorInit() {
    Panic("globalStableAllocatorInit is not implemented");
}

static i32 getFreeBlockIndex() {
    Panic("getFreeBlockIndex is not implemented");
}

static StableBlock *getStableBlock(i32 index) {
    Panic("getStableBlock is not implemented");
}