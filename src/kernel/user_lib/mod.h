#pragma once

#include "../lib/mod.h"

void SyscallTest();
void SyscallPrint(String message, Size len);
u32 Fork();
void Yield();
Size Read(String buf, Size len);
PID GetPID();