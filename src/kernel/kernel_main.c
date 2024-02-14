#include "common/mod.h"
#include "lib/mod.h"
#include "console/mod.h"

void KernelMain() {
    // 清空屏幕
    InitializeConsole();

    // 使用可变参数列表实现的Printf
    Printf("Hello, World! This is OS Printf\n");
}