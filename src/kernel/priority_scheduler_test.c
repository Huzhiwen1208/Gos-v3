#include "common/type.h"
#include "console/type.h"
#include "lib/method.h"
#include "process/type.h"
#include "process/method.h"

#define PRIORITY_TEST_PROCESS_COUNT 3

static ProcessPriority executionOrder[PRIORITY_TEST_PROCESS_COUNT];
static u32 executionCount;

static void recordExecution(ProcessPriority priority);
static void blockCurrentTestProcess();
static void lowPriorityProcess();
static void normalPriorityProcess();
static void highPriorityProcess();

/*
 * 以低、高、普通的顺序入队。严格优先级调度应实际运行：高、普通、低。
 * 每个测试进程记录一次后阻塞，避免高优先级进程重复运行导致低优先级饥饿。
 */
void RunPrioritySchedulerTest() {
    executionCount = 0;
    PrintWithColor(LIGHT_BLUE, "[TEST] priority scheduler\n");

    CreateKernelProcessWithPriority(lowPriorityProcess, PROCESS_PRIORITY_LOW);
    CreateKernelProcessWithPriority(highPriorityProcess, PROCESS_PRIORITY_HIGH);
    CreateKernelProcessWithPriority(normalPriorityProcess, PROCESS_PRIORITY_NORMAL);
}

static void highPriorityProcess() {
    recordExecution(PROCESS_PRIORITY_HIGH);
    blockCurrentTestProcess();
}

static void normalPriorityProcess() {
    recordExecution(PROCESS_PRIORITY_NORMAL);
    blockCurrentTestProcess();
}

static void lowPriorityProcess() {
    recordExecution(PROCESS_PRIORITY_LOW);

    Boolean passed = executionCount == PRIORITY_TEST_PROCESS_COUNT
        && executionOrder[0] == PROCESS_PRIORITY_HIGH
        && executionOrder[1] == PROCESS_PRIORITY_NORMAL
        && executionOrder[2] == PROCESS_PRIORITY_LOW;
    PrintWithColor(passed ? GREEN : LIGHT_RED,
                   "  %s: priority order is high -> normal -> low\n",
                   passed ? "PASS" : "FAIL");
    blockCurrentTestProcess();
}

static void recordExecution(ProcessPriority priority) {
    if (executionCount < PRIORITY_TEST_PROCESS_COUNT) {
        executionOrder[executionCount++] = priority;
    }
}

static void blockCurrentTestProcess() {
    GetCurrentProcess()->Status = PROCESS_STATE_BLOCKED;
    Schedule();
    while (TRUE) {
    }
}
