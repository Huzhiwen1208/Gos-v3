#include "mod.h"

/* src/kernel/process/create.c */
#include "mod.h"

/// @brief 创建内核级进程
/// @param entry: 进程入口地址
void CreateKernelProcess(void *entry) {
    PCB *process = (PCB *)Malloc(sizeof(PCB));
    process->ID = AllocatePID();
    process->Status = PROCESS_STATE_RUNNABLE;
    u32 stack = AllocateOnePage(KernelMode) + PageSize;
    process->Type = PROCESS_TYPE_KERNEL;

    stack -= sizeof(SwitchContext);
    SwitchContext *context = (SwitchContext *)stack;
    context->EIP = (u32)entry;
    context->EBP = 0;
    context->ESI = 0;
    context->EDI = 0;
    context->EBX = 0;
    process->KernelStackPointer = (PhysicalAddress *)stack;

    AddProcess(process);
}