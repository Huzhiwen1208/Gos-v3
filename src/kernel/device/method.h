#pragma once
#include "../common/type.h"
#include "type.h"

Device* GetDeviceByID(u32 deviceID);
void InitializeDevice();
u32 InstallDevice(Device* device);
u32 UninstallDevice(Device* device);
void DeviceIoctl(u32 deviceID, u32 cmd, void *arg);
i32 DeviceRead(u32 deviceID, u64 offset, u32 size, void *buffer);
i32 DeviceWrite(u32 deviceID, u64 offset, u32 size, void *buffer);