#include "mod.h"

static VirtualAddress getPageFaultAddress() {
    asm volatile ("movl %cr2, %eax");
}

// TODO: 实现缺页异常处理
void InitializePageFaultHandler() {
    
}