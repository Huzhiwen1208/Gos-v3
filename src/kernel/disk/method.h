#pragma once
#include "../common/type.h"
#include "type.h"

void InitializeIdeDisk();
void DiskCacheRead(u32 blockID, void* buffer);
void DiskCacheWrite(u32 blockID, void* buffer);
void InitializeDiskCache();