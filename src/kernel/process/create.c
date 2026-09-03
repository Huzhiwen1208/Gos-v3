#include "mod.h"
#include "../lib/method.h"
#include "../memory/method.h"

void CreateKernelProcessWithPriority(void* entry, ProcessPriority priority) {
    PCB* process = (PCB*)Malloc(sizeof(PCB));
    process->ID = AllocatePID();
    process->Status = PROCESS_STATE_RUNNABLE;
    u32 stack = AllocateOnePage(KernelMode) + PageSize;
    process->Type = PROCESS_TYPE_KERNEL;
    process->Priority = priority;

    stack -= sizeof(SwitchContext);
    SwitchContext* context = (SwitchContext*)stack;
    context->EIP = (u32)entry;
    context->EBP = 0;
    context->ESI = 0;
    context->EDI = 0;
    context->EBX = 0;
    process->KernelStackPointer = (PhysicalAddress*)stack;

    AddProcess(process);
}

void CreateKernelProcess(void* entry) {
    CreateKernelProcessWithPriority(entry, PROCESS_PRIORITY_NORMAL);
}
