#include "mod.h"

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

void SyscallPrint(String message, Size len) {
    SystemCall(SYSCALL_WRITE, (u32)message, (u32)len, 0);
}

u32 Fork() {
    return SystemCall(SYSCALL_FORK, 0, 0, 0);
}

void Yield() {
    SystemCall(SYSCALL_YIELD, 0, 0, 0);
}

Size Read(String buf, Size len) {
    return SystemCall(SYSCALL_READ, (u32)buf, (u32)len, 0);
}

PID GetPID() {
    return SystemCall(SYSCALL_GET_PID, 0, 0, 0);
}