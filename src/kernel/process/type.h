#pragma once

#include "../common/type.h"

typedef enum ProcessState {
    PROCESS_STATE_RUNNABLE, // 就绪态
    PROCESS_STATE_RUNNING, // 运行态
    PROCESS_STATE_BLOCKED // 阻塞态
} ProcessState;

typedef enum ProcessType {
    PROCESS_TYPE_USER, // 用户态进程
    PROCESS_TYPE_KERNEL // 内核态进程
} ProcessType;

typedef struct PCB {
    PhysicalAddress *KernelStackPointer;  // 进程内核栈栈顶指针
    PID ID; // 进程ID
    ProcessState Status; // 进程当前状态
    ProcessType Type; // 进程类型
} PCB;

#define MAX_PROCESS_COUNT 1024 // 最大进程数量

/// @brief 进程ID分配器
typedef struct PIDAllocator {
    u32 Bitmap[MAX_PROCESS_COUNT / 32];
} PIDAllocator;

// Process Manager
typedef struct ProcessManager {
    PCB *Current; // 当前进程
    PCB *RunnableProcesses[MAX_PROCESS_COUNT]; // 就绪态进程队列
} ProcessManager;

// Process Need

#define PageSize 4096 // 内存页大小

// Context
typedef struct SwitchContext {
    u32 EDI;
    u32 ESI;
    u32 EBX;
    u32 EBP;
    u32 EIP;
} SwitchContext;