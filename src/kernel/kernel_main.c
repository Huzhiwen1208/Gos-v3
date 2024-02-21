#include "common/mod.h"
#include "lib/mod.h"
#include "console/mod.h"
#include "memory/mod.h"
#include "int/mod.h"

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
    *   2.1 录制异常处理视频
    *   2.2 实现新的时钟中断处理，并打开时钟中断、CPU中断，此处参考视频中的讲解
    *   2.3 实现一个系统调用
    */
    SyscallLab2();
}