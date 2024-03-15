#include "mod.h"

static void syscallTest(u32 arg1, u32 arg2, u32 arg3) {
    Info("syscall test: arg{%d, %d, %d}", arg1, arg2, arg3);
}

static void syscallWrite(String message, Size len) {
    ConsoleWriteWithColor(message, len, WHITE);
}

static PID syscallFork() {
    return ForkProcess();
}

static void syscallYield() {
    Schedule();
}

static Size syscallRead(String buf, Size len) {
    return ReadLine(buf, len);
}

static PID syscallGetPid() {
    return GetCurrentProcess()->ID;
}

u32 TrapHandler(u32 syscallNum, u32 arg1, u32 arg2, u32 arg3) {
    switch (syscallNum) {
        case SYSCALL_TEST:
            syscallTest(arg1, arg2, arg3);
            break;
        case SYSCALL_WRITE:
            syscallWrite((String)arg1, (Size)arg2);
            break;
        case SYSCALL_FORK:
            return syscallFork();
        case SYSCALL_YIELD:
            syscallYield();
            break;
        case SYSCALL_READ:
            return syscallRead(arg1, arg2);
        case SYSCALL_GET_PID:
            return syscallGetPid();
        default:
            Panic("Unknown syscall number: %d", syscallNum);
    }

    return 0;
}