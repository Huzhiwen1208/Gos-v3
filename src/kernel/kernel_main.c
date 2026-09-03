#include "common/mod.h"
#include "console/method.h"
#include "console/type.h"
#include "lib/method.h"
#include "memory/mod.h"
#include "int/mod.h"

static void clockInterruptHandler(u32 vector) {
    PrintWithColor(GREEN, "The clock interrupt was trigerred!\n");
}

void KernelMain() {
    // 清空屏幕，初始化控制台
    InitializeConsole();

    // 使用可变参数列表实现的Printf
    Printf("Hello, World! This is OS Printf\n");

    // 初始化内存管理
    InitMemoryManager();

    // 初始化中断异常处理
    InitializeInterrupt();

    /* TODO 实验：
    *   2.1 实现新的时钟中断处理，并打开时钟中断、CPU中断，此处可参考文档中的内容
    *   2.2 实现一个系统调用 SyscallLab2
    */

    SetInterruptHandler(0x20, clockInterruptHandler);
    SetInterrupt(0x20);
    asm volatile ("sti");

    SyscallLab2();
}
