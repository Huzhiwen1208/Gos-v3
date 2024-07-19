#include "method.h"
#include "type.h"
#include "../console/method.h"
#include "../fs/method.h"
#include "../fs/type.h"
#include "../lib/method.h"
#include "../process/method.h"

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

static String syscallPWD() {
    return GetCurrentPath();
}

static void syscallListDir(String path, String option) {
    ListFiles(option, path);
}

static void syscallCreateFile(String name) {
    CreateFile(-1, name, FT_FILE);
}

static char syscallReadChar() {
    return ReadChar();
}

static void syscallWriteFile(String content, String filename) {
    WriteFileContent(filename, content, TRUE);
}

static void syscallCatFile(String filename) {
    PrintFileContent(filename, 1);
}

u32 TrapHandler(u32 syscallNum, u32 arg1, u32 arg2, u32 arg3) {
    switch (syscallNum) {
        case SYSCALL_TEST:
            syscallTest(arg1, arg2, arg3);
            break;
        case SYSCALL_WRITE:
            syscallWrite((String)arg1, (Size)arg2);
            break;
        case SYSCALL_FORK:
            return syscallFork();
        case SYSCALL_YIELD:
            syscallYield();
            break;
        case SYSCALL_READ:
            return syscallRead((String)arg1, arg2);
        case SYSCALL_READ_CHAR:
            return syscallReadChar();
        case SYSCALL_GET_PID:
            return syscallGetPid();
        case SYSCALL_GET_TIME:
            return syscallGetTime();
        case SYSCALL_EXIT:
            syscallExit(arg1);
            break;
        case SYSCALL_WAIT_PID:
            return syscallWaitPid(arg1, (i32*)arg2);
        case SYSCALL_PWD:
            return (u32)syscallPWD();
        case SYSCALL_LS:
            syscallListDir((String)arg1, (String)arg2);
            break;
        case SYSCALL_CREATE_FILE:
            syscallCreateFile((String)arg1);
            break;
        case SYSCALL_WRITE_FILE:
            syscallWriteFile((String)arg1, (String)arg2);
            break;
        case SYSCALL_CAT_FILE:
            syscallCatFile((String)arg1);
            break;
        default:
            Panic("Unknown syscall number: %d", syscallNum);
    }

    return 0;
}
