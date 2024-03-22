#pragma once
void InitializeIdeDisk();
void DiskCacheRead(u32 blockID, void* buffer);
void DiskCacheWrite(u32 blockID, void* buffer);