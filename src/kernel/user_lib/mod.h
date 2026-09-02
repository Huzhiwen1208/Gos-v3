#ifndef GOS_KERNEL_USER_LIB_MOD_H
#define GOS_KERNEL_USER_LIB_MOD_H

#include "../lib/mod.h"

void SyscallTest();

#endif
void SyscallPrint(String message, Size len);
u32 Fork();
void Yield();
Size Read(String buf, Size len);
PID GetPID();
u32 GetTime();
void Exit(i32 exitCode);
PID WaitPid(PID pid, i32* exitCode);
