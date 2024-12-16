#include "mod.h"

/// @brief 用户态系统调用统一入口
/// @param syscallNum 
/// @param arg1 
/// @param arg2 
/// @param arg3 
/// @return 
static u32 SystemCall(u32 syscallNum, u32 arg1, u32 arg2, u32 arg3) {
    asm volatile ("movl %0, %%eax" : : "m"(syscallNum));
    asm volatile ("movl %0, %%ebx" : : "m"(arg1));
    asm volatile ("movl %0, %%ecx" : : "m"(arg2));
    asm volatile ("movl %0, %%edx" : : "m"(arg3));
    asm volatile ("int $0x80");
}

void SyscallTest() {
    SystemCall(SYSCALL_TEST, 1, 2, 3);
}