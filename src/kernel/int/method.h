#pragma once

#include "mod.h"

void InitializeInterrupt();
void SetInterruptHandler(u32 vector, void* handler);
void SetInterrupt(u32 vector);
void OuteralInterruptCompleted(u32 vector);
u8 GetInterruptStatus();
void RestoreInterruptStatus(u8 status);

void InitializePageFaultHandler();

void InitializeClock();
u32 GetTimeMS();