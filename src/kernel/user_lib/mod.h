#pragma once

#include "../lib/mod.h"

void SyscallTest();
void SyscallPrint(String message, Size len);
u32 Fork();
void Yield();
Size Read(String buf, Size len);
PID GetPID();
u32 GetTime();
void Exit(i32 exitCode);
PID WaitPid(PID pid, i32* exitCode);