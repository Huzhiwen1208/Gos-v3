#include "mod.h"

static ProcessManager processManager;
static PIDAllocator pidAllocator;

static PCB* fetchProcess();
static void runFirstProcess();
static PCB* idleProcess;
static void idle();
static Boolean isEmpty();

// public methods

void InitializeProcessManager() {
    processManager.Current = NULL;
    for (int i = 0; i < MAX_PROCESS_COUNT; i++) {
        processManager.RunnableProcesses[i] = NULL;
    }
    CreateKernelProcess(idle);
    idleProcess = fetchProcess();
}

PID AllocatePID() {
    for (i32 i = 0; i < MAX_PROCESS_COUNT / 32; i++) {
        for (i32 j = 0; j < 32; j++) {
            if ((pidAllocator.Bitmap[i] & (1 << j)) == 0) {
                pidAllocator.Bitmap[i] |= (1 << j);
                return i * 32 + j;
            }
        }
    }
}

void FreePID(PID pid) {
    pidAllocator.Bitmap[pid / 32] &= ~(1 << (pid % 32));
}

PCB* GetCurrentProcess() {
    return processManager.Current;
}

void AddProcess(PCB* process) {
    for (int i = 0; i < MAX_PROCESS_COUNT; i++)
        if (processManager.RunnableProcesses[i] == NULL) {
            processManager.RunnableProcesses[i] = process;
            return;
        }
    
    Panic("Too many processes");
}

void Schedule() {
    // 如果就绪队列里没有进程，并且当前进程也不存在，说明还未初始化进程管理器，直接退出即可
    if (processManager.Current == NULL && isEmpty()) {
        return;
    }

    // 如果就绪队列里有进程，但是当前进程不存在，说明可以执行第一个进程
    if (processManager.Current == NULL) {
        runFirstProcess();
        return;
    }
    
    // 如果就绪队列里有进程，并且当前进程存在，说明可以执行切换
    PCB* current = processManager.Current;
    if (current->ID && current->Status != PROCESS_STATE_BLOCKED) {
        current->Status = PROCESS_STATE_RUNNABLE;
        AddProcess(current);
    }

    PCB* next = fetchProcess();
    next->Status = PROCESS_STATE_RUNNING;

    processManager.Current = next;
    SwitchProcess(current, next);
}

// static methods implement

static void runFirstProcess() {
    PCB* next = fetchProcess();
    next->Status = PROCESS_STATE_RUNNING;

    PCB unused;
    PCB* unusedPtr = &unused;
    processManager.Current = next;
    SwitchProcess(unusedPtr, next);
}

static PCB* fetchProcess() {
    for (int i = 0; i < MAX_PROCESS_COUNT; i++) {
        if (processManager.RunnableProcesses[i] != NULL) {
            PCB* process = processManager.RunnableProcesses[i];
            processManager.RunnableProcesses[i] = NULL;
            return process;
        }
    }
    return NULL;
}

static void idle() {
    while (TRUE) {
        Schedule();
    }
}

static Boolean isEmpty() {
    for (int i = 0; i < MAX_PROCESS_COUNT; i++) {
        if (processManager.RunnableProcesses[i] != NULL) {
            return FALSE;
        }
    }
    return TRUE;
}