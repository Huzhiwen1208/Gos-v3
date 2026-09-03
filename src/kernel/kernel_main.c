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
#include "gdt/method.h"

extern void user_process();

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

    // 初始化全局描述符，TSS
    InitializeGDT();

    CreateUserProcess(user_process);
    Schedule();
}
