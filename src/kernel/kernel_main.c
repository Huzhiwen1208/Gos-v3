#include "common/mod.h"
#include "lib/mod.h"
#include "console/mod.h"
#include "memory/mod.h"

void KernelMain() {
    // 清空屏幕，初始化控制台
    InitializeConsole();

    // 使用可变参数列表实现的Printf
    Printf("Hello, World! This is OS Printf\n");

    // 初始化内存管理
    InitMemoryManager();

    // 测试BuddyAllocator
    testBuddyAllocator();

    // 实验1内容如下，在完成实验1后，修改done的值为True
    Boolean done = TRUE;
    if (done) {
        // 1. 测试日志信息
        Debug("This is Debug");
        Trace("This is Trace");
        Info("This is Info");
        Warn("This is Warn");
        Error("This is Error\n");

        // // 2. 测试StableAllocator
        testStableAllocator();
    }
}