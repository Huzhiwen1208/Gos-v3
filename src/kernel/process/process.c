#include "mod.h"
#include "../lib/method.h"
#include "../memory/method.h"
#include "../process/method.h"

extern void SwitchProcess(PCB *current, PCB *next);

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
    processManager.Front = processManager.Rear = 0;
    CreateKernelProcessWithPriority(idle, PROCESS_PRIORITY_IDLE);
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

/// @brief 入队，进入就绪队列
/// @param process 
void AddProcess(PCB* process) {
    if ((processManager.Rear + 1) % MAX_PROCESS_COUNT == processManager.Front) {
        Panic("Queue is full");
    }

    processManager.RunnableProcesses[processManager.Rear] = process;
    processManager.Rear = (processManager.Rear + 1) % MAX_PROCESS_COUNT;
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

    if (current == idleProcess && isEmpty()) {
        return;
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
    if (isEmpty()) {
        return idleProcess;
    }

    /*
     * 在环形就绪队列中选择优先级最高的进程。
     * 从队首开始扫描；同一优先级不覆盖已选项，因此保持 FIFO 轮转。
     */
    u32 selected = processManager.Front;
    for (u32 index = (processManager.Front + 1) % MAX_PROCESS_COUNT;
         index != processManager.Rear;
         index = (index + 1) % MAX_PROCESS_COUNT) {
        if (processManager.RunnableProcesses[index]->Priority
                > processManager.RunnableProcesses[selected]->Priority) {
            selected = index;
        }
    }

    PCB* result = processManager.RunnableProcesses[selected];

    /* 删除选中项并保持其余就绪进程的相对入队顺序。 */
    u32 last = (processManager.Rear + MAX_PROCESS_COUNT - 1) % MAX_PROCESS_COUNT;
    for (u32 index = selected; index != last; index = (index + 1) % MAX_PROCESS_COUNT) {
        u32 next = (index + 1) % MAX_PROCESS_COUNT;
        processManager.RunnableProcesses[index] = processManager.RunnableProcesses[next];
    }
    processManager.RunnableProcesses[last] = NULL;
    processManager.Rear = last;

    return result;
}

static void idle() {
    while (TRUE) {
        Schedule();
    }
}

static Boolean isEmpty() {
    return processManager.Front == processManager.Rear;
}
