#pragma once
void InitializeIdeDisk();
void InitializeDiskCache();
void DiskCacheRead(u32 blockID, void *buffer);
void DiskCacheWrite(u32 blockID, void *buffer);