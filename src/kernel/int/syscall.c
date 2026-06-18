#include "mod.h"
#include "../lib/method.h"

static void syscallTest(u32 arg1, u32 arg2, u32 arg3) {
    Info("syscall test: arg{%d, %d, %d}", arg1, arg2, arg3);
}

/// @brief 系统调用 C 入口函数
/// @param syscallNum 
/// @param arg1 
/// @param arg2 
/// @param arg3 
/// @return 
u32 TrapHandler(u32 syscallNum, u32 arg1, u32 arg2, u32 arg3) {
    switch (syscallNum) {
    case SYSCALL_TEST:
        syscallTest(arg1, arg2, arg3);
        break;
    default:
        Panic("Unknown syscall number: %d", syscallNum);
    }

    return 0;
}