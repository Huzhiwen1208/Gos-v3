#include "common/mod.h"
#include "lib/mod.h"
#include "console/mod.h"
#include "memory/mod.h"
#include "int/mod.h"
#include "process/mod.h"

void KernelMainTest();

void KernelMain() {
    // 清空屏幕，初始化控制台
    InitializeConsole();
    // 初始化内存管理
    InitMemoryManager();
    // 初始化中断异常处理
    InitializeInterrupt();
    // 初始化进程管理
    InitializeProcessManager();
    // 初始化全局描述符，TSS
    InitializeGDT();
    // 初始化虚拟分页
    InitializeMemoryMapping();
    // 初始化异常处理
    InitializePageFaultHandler();
    // 初始化时钟中断
    InitializeClock();
    // 初始化键盘中断
    InitializeKeyboard();

    KernelMainTest();
}
