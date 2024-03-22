#include "mod.h"

static HashTable* diskCacheMap; // blockID: PhysicalAddress

void InitializeDiskCache() {
    diskCacheMap = NewMap("u32", "PhysicalAddress");
}

void DiskCacheRead(u32 blockID, void* buffer) {
    PhysicalAddress addr = (PhysicalAddress)diskCacheMap->Get(diskCacheMap, blockID);
    // cache miss
    if (addr == NULL) {
        DeviceRead(FS_DEVICE, blockID, 1, buffer);
        diskCacheMap->Put(diskCacheMap, blockID, buffer); // 更新缓存
        return;
    }

    // cache hit
    MemoryCopy(buffer, addr, 512);
}

// 全写法
void DiskCacheWrite(u32 blockID, void* buffer) {
    PhysicalAddress addr = (PhysicalAddress)diskCacheMap->Get(diskCacheMap, blockID);
    if (addr) {
        MemoryCopy(addr, buffer, 512);
        diskCacheMap->Put(diskCacheMap, blockID, buffer);
    }
    DeviceWrite(FS_DEVICE, blockID, 1, buffer);
}