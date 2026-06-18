#include "mod.h"
#include "../lib/method.h"
#include "../memory/method.h"
#include "../process/method.h"

extern void SwitchProcess(PCB *current, PCB *next);

static ProcessManager processManager;
static PIDAllocator pidAllocator;

static PCB *fetchProcess();
static void runFirstProcess();
static PCB *idleProcess;
static void idle();
static Boolean isEmpty();

// public methods

/// @brief 初始化进程管理器
void InitializeProcessManager() {
    processManager.Current = NULL;
    for (int i = 0; i < MAX_PROCESS_COUNT; i++) {
        processManager.RunnableProcesses[i] = NULL;
    }
    CreateKernelProcess(idle);
    idleProcess = fetchProcess();
}

/// @brief 分配一个进程ID
/// @return 
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

/// @brief 释放一个进程ID
/// @param pid 
void FreePID(PID pid) {
    pidAllocator.Bitmap[pid / 32] &= ~(1 << (pid % 32));
}

/// @brief 获取当前进程的 PCB 指针
/// @return 
PCB *GetCurrentProcess() {
    return processManager.Current;
}

/// @brief 添加一个进程到就绪队列，一般用于进程创建后或进程被唤醒后
/// @param process 
void AddProcess(PCB *process) {
    for (int i = 0; i < MAX_PROCESS_COUNT; i++)
        if (processManager.RunnableProcesses[i] == NULL) {
            processManager.RunnableProcesses[i] = process;
            return;
        }

    Panic("Too many processes");
}

/// @brief 进程调度器
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
    PCB *current = processManager.Current;
    if (current->ID && current->Status != PROCESS_STATE_BLOCKED) {
        current->Status = PROCESS_STATE_RUNNABLE;
        AddProcess(current);
    }

    PCB *next = fetchProcess();
    next->Status = PROCESS_STATE_RUNNING;

    processManager.Current = next;
    SwitchProcess(current, next);
}

// static methods implement

/// @brief 执行第一个进程
static void runFirstProcess() {
    PCB *next = fetchProcess();
    next->Status = PROCESS_STATE_RUNNING;

    PCB unused;
    PCB *unusedPtr = &unused;
    processManager.Current = next;
    SwitchProcess(unusedPtr, next);
}

/// @brief 选取下一个要执行的进程上处理机
/// @return 
static PCB *fetchProcess() {
    for (int i = 0; i < MAX_PROCESS_COUNT; i++) {
        if (processManager.RunnableProcesses[i] != NULL) {
            PCB *process = processManager.RunnableProcesses[i];
            processManager.RunnableProcesses[i] = NULL;
            return process;
        }
    }
    return NULL;
}

/// @brief 空闲进程
static void idle() {
    while (TRUE) {
        Schedule();
    }
}

/// @brief 判断进程就绪队列是否为空
/// @return 
static Boolean isEmpty() {
    for (int i = 0; i < MAX_PROCESS_COUNT; i++) {
        if (processManager.RunnableProcesses[i] != NULL) {
            return FALSE;
        }
    }
    return TRUE;
}