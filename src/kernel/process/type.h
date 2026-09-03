#pragma once

#include "../common/type.h"

typedef enum ProcessState {
    PROCESS_STATE_RUNNABLE,
    PROCESS_STATE_RUNNING,
    PROCESS_STATE_BLOCKED
} ProcessState;

typedef enum ProcessType {
    PROCESS_TYPE_USER,
    PROCESS_TYPE_KERNEL
} ProcessType;

/* 数值越大，调度优先级越高。 */
typedef enum ProcessPriority {
    PROCESS_PRIORITY_IDLE = 0,
    PROCESS_PRIORITY_LOW,
    PROCESS_PRIORITY_NORMAL,
    PROCESS_PRIORITY_HIGH,
    PROCESS_PRIORITY_REALTIME
} ProcessPriority;

typedef struct PCB {
    PhysicalAddress* KernelStackPointer;
    PID ID;
    ProcessState Status;
    ProcessType Type;
    ProcessPriority Priority;
} PCB;

// PID allocator
#define MAX_PROCESS_COUNT 1024
typedef struct PIDAllocator {
    u32 Bitmap[MAX_PROCESS_COUNT / 32];
} PIDAllocator;

// Process Manager
typedef struct ProcessManager {
    PCB* Current;
    PCB* RunnableProcesses[MAX_PROCESS_COUNT];
    u32 Front;
    u32 Rear;
} ProcessManager;

// Process Need

#define PageSize 4096

// Context
typedef struct SwitchContext {
    u32 EDI;
    u32 ESI;
    u32 EBX;
    u32 EBP;
    u32 EIP;
} SwitchContext;
