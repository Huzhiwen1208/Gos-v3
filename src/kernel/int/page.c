#include "mod.h"
#include "../lib/method.h"

static VirtualAddress getPageFaultAddress() {
    asm volatile ("movl %cr2, %eax");
}

static void pageFaultHander(u32 vector) {
    Assert(vector == 0xe);

    VirtualAddress addr = getPageFaultAddress();
    if (addr == 0) {
        Panic("Null pointer exception");
    }

    if (addr < 0x400000) {
        Panic("System terminated");
    }

    MapPage(addr); // 把虚地址addr映射了
}

// TODO: 实现缺页异常处理
void InitializePageFaultHandler() {
    SetInterruptHandler(0x0E, pageFaultHander);
}