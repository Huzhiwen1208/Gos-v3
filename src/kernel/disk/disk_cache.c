#include "mod.h"

static HashTable *diskCacheMap; // blockID: PhysicalAddress

void InitializeDiskCache() {
    diskCacheMap = NewMap("u32", "PhysicalAddress");
}

void DiskCacheRead(u32 blockID, void *buffer) {
    PhysicalAddress addr = (PhysicalAddress)diskCacheMap->Get(diskCacheMap, blockID);
    // cache miss
    if (addr == NULL) {
        void *new_buffer = (void *)Malloc(512 * sizeof(char));
        DeviceRead(FS_DEVICE, blockID, 1, new_buffer);
        diskCacheMap->Put(diskCacheMap, blockID, new_buffer); // 更新缓存
        MemoryCopy(buffer, new_buffer, 512);
        return;
    }

    // cache hit
    MemoryCopy(buffer, addr, 512);
}

// 全写法
void DiskCacheWrite(u32 blockID, void *buffer) {
    PhysicalAddress addr = (PhysicalAddress)diskCacheMap->Get(diskCacheMap, blockID);
    // cache miss
    if (addr == NULL) {
        DeviceWrite(FS_DEVICE, blockID, 1, buffer);
        return;
    }

    // cache hit
    MemoryCopy(addr, buffer, 512);
    DeviceWrite(FS_DEVICE, blockID, 1, buffer);
}