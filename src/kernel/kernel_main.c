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

    SyscallTest();
}