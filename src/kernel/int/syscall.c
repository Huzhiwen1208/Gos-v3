#include "mod.h"

static void syscallTest(u32 arg1, u32 arg2, u32 arg3) {
    Info("syscall test: arg{%d, %d, %d}", arg1, arg2, arg3);
}

static void syscallWrite(String message, Size len) {
    ConsoleWriteWithColor(message, len, WHITE);
}

static PID syscallFork() {
    return ForkProcess();
}

static void syscallYield() {
    Schedule();
}

static Size syscallRead(String buf, Size len) {
    return ReadLine(buf, len);
}

static PID syscallGetPid() {
    return GetCurrentProcess()->ID;
}

static u32 syscallGetTime() {
    return GetTimeMS();
}

static void syscallExit(i32 exitCode) {
    ExitProcess(exitCode);
}

static PID syscallWaitPid(PID pid, i32* exitCode) {
    return WaitProcess(pid, exitCode);
}

static PID syscallGetPPID() {
    return GetCurrentProcess()->ParentID;
}

static void syscallTaskInfo(TaskInfo* info) {
    PCB* current = GetCurrentProcess();
    info->pid = current->ID;
    info->status = current->Status;

    int current_time = GetTimeMS(); // 当前时间
    int create_time = current->CreateTime; // 创建时间
    info->time = current_time - create_time;

    for (int i = 0; i < MAX_SYSCALL_NUM; i++) {
        info->call[i] = current->SyscallTimes[i];
    }

    info->call[SYSCALL_TASK_INFO]++;
}

static void syscallSleep(u32 sleepTime) {
    GetCurrentProcess()->WackupTime = GetTimeMS() + sleepTime;
    SleepProcess(GetCurrentProcess());
}

static i32 syscallMmap(u32 start, u32 len, u32 prot) {
    if (start % PageSize != 0) {
        Panic("start address must be page aligned");
    }

    if (prot &!0x7 != 0) {
        Panic("invalid prot flags");
    }

    if (prot & 0x7 == 0) {
        Panic("prot flags must be set");
    }

    for (u32 address = start; address < start + len; address += PageSize) {
        if (IsMapped(address)) {
            Panic("address 0x%x is already mapped", address);
        }

        MapPage(address);
    }

    return 0;
}

static i32 syscallMunmap(u32 start, u32 len) {
    if (start % PageSize != 0) {
        Panic("start address must be page aligned");
    }

    for (u32 address = start; address < start + len; address += PageSize) {
        if (!IsMapped(address)) {
            Panic("address 0x%x is not mapped", address);
        }

        UnmapPage(address);
    }

    return 0;
}

u32 TrapHandler(u32 syscallNum, u32 arg1, u32 arg2, u32 arg3) {
    u32 result = 0;
    switch (syscallNum) {
        case SYSCALL_TEST:
            syscallTest(arg1, arg2, arg3);
            break;
        case SYSCALL_WRITE:
            syscallWrite((String)arg1, (Size)arg2);
            break;
        case SYSCALL_FORK:
            result = syscallFork();
            break;
        case SYSCALL_YIELD:
            syscallYield();
            break;
        case SYSCALL_READ:
            result = syscallRead(arg1, arg2);
            break;
        case SYSCALL_GET_PID:
            result = syscallGetPid();
            break;
        case SYSCALL_GET_TIME:
            result = syscallGetTime();
            break;
        case SYSCALL_EXIT:
            syscallExit(arg1);
            break;
        case SYSCALL_WAIT_PID:
            result = syscallWaitPid(arg1, arg2);
            break;
        case SYSCALL_GET_PPID:
            result = syscallGetPPID();
            break;
        case SYSCALL_TASK_INFO:
            syscallTaskInfo((TaskInfo*)arg1);
            return;
        case SYSCALL_SLEEP:
            syscallSleep(arg1);
            break;
        case SYSCALL_MMAP:
            result = syscallMmap(arg1, arg2, arg3);
            break;
        case SYSCALL_MUNMAP:
            result = syscallMunmap(arg1, arg2);
            break;
        default:
            Panic("Unknown syscall number: %d", syscallNum);
    }

    PCB* current = GetCurrentProcess();
    current->SyscallTimes[syscallNum]++;
    return result;
}