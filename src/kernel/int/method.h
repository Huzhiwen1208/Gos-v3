#pragma once
#include "../common/type.h"
#include "type.h"

void InitializeInterrupt();
void InitializeIDT();
void SetInterruptHandler(u32 vector, void* handler);
void SetInterrupt(u32 vector);
void OuteralInterruptCompleted(u32 vector);
u8 GetInterruptStatus();
void RestoreInterruptStatus(u8 status);

void InitializePageFaultHandler();

// 时钟
void InitializeClock();
u32 GetTimeMS();

// 键盘
void InitializeKeyboard();
Size ReadLine(char* buf, Size len);
char ReadChar();
