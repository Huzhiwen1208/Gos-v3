#include "common/mod.h"
#include "console/type.h"
#include "console/method.h"
#include "lib/method.h"
#include "memory/type.h"
#include "memory/method.h"
#include "int/type.h"
#include "int/method.h"
#include "process/type.h"
#include "process/method.h"

void testProcess() {
    Printf("Hello, World! This is a test process\n");
    while (1);
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

    // 初始化进程管理
    InitializeProcessManager();
    CreateKernelProcess(testProcess);
    // TODO: 1. 找到调度器的BUG，检查 AddProcess 和 fetchProcess 方法
    Schedule();
    /* TODO: 2. 创建三个进程A、B、C。
        1. A进程打印“I am A process”后调度执行。
        2. B进程打印“I am B process”后调度执行。
        3. C进程打印“I am C process”后调度执行。
    */
}
