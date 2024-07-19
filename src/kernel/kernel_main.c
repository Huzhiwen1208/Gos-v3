#include "common/method.h"
#include "console/method.h"
#include "memory/method.h"
#include "int/method.h"
#include "gdt/method.h"
#include "process/method.h"
#include "device/method.h"
#include "disk/method.h"
#include "fs/method.h"
#include "ds/method.h"

extern void GotoGshell();

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
    // 初始化设备表
    InitializeDevice();
    // 初始化磁盘
    InitializeIdeDisk();
    // 初始化文件系统
    InitializeFileSystem();

    // KernelMainTest();
    GotoGshell();
}