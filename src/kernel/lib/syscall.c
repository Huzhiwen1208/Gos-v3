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

u32 GetTime() {
    return SystemCall(SYSCALL_GET_TIME, 0, 0, 0);
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

void Exit(i32 exitCode) {
    SystemCall(SYSCALL_EXIT, (u32)exitCode, 0, 0);
}

PID WaitPid(PID pid, i32* exitCode) {
    while (TRUE) {
        PID rid = SystemCall(SYSCALL_WAIT_PID, (u32)pid, (u32)exitCode, 0);
        switch (rid) {
            case -2:
                Yield();
                break;
            default:
                return rid;
        }
    }
}

PID GetPPID() {
    SystemCall(SYSCALL_GET_PPID, 0, 0, 0);
}

// TODO: implement the following functions
void GetTaskInfo(TaskInfo* info) {
    SystemCall(SYSCALL_TASK_INFO, (u32)((void*)info), 0, 0);
}

i32 MMap(u32 start, u32 len, u32 prot) {
    while (1) {
        for (int i = 0; i < 100000000; i++); // 类似于sleep作用
        printf("MMap Not implemented yet\n");
    }
}

i32 MUnmap(u32 start, u32 len) {
    while (1) {
        for (int i = 0; i < 100000000; i++); // 类似于sleep作用
        printf("MUnmap Not implemented yet\n");
    }
}

void Sleep(u32 time) {
    SystemCall(SYSCALL_SLEEP, time, 0, 0);
}