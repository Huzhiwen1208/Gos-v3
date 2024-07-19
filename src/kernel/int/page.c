#include "method.h"
#include "type.h"
#include "../lib/method.h"
#include "../memory/method.h"

static VirtualAddress getCR2() {
    asm volatile ("movl %cr2, %eax");
}

static void pageFaultHandler(u32 vector) {
    Assert(vector == 0xe);

    VirtualAddress addr = getCR2();
    if (addr == NULL) {
        Panic("Access null pointer");
    } 
    // Warn("Page fault at 0x%x\n", addr);
    MapPage(addr);
}

void InitializePageFaultHandler() {
    SetInterruptHandler(0xe, pageFaultHandler);
}
