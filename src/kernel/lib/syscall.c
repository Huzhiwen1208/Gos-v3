#include "mod.h"

static u32 SystemCall(u32 syscallNum, u32 arg1, u32 arg2, u32 arg3) {
    u32 result;
    asm volatile (
        "int $0x80"
        : "=a"(result)
        : "a"(syscallNum), "b"(arg1), "c"(arg2), "d"(arg3)
        : "cc", "memory"
    );
    return result;
}

void SyscallTest() {
    SystemCall(SYSCALL_TEST, 1, 2, 3);
}

void SyscallPrint(String message, Size len) {
    SystemCall(SYSCALL_WRITE, (u32)message, (u32)len, 0);
}

u32 Fork() {
    return SystemCall(SYSCALL_FORK, 0, 0, 0);
}

u32 GetTime() {
    return SystemCall(SYSCALL_GET_TIME, 0, 0, 0);
}

void Yield() {
    SystemCall(SYSCALL_YIELD, 0, 0, 0);
}

Size Read(String buf, Size len) {
    return SystemCall(SYSCALL_READ, (u32)buf, (u32)len, 0);
}

char GetChar() {
    return SystemCall(SYSCALL_READ_CHAR, 0, 0, 0);
}

PID GetPID() {
    return SystemCall(SYSCALL_GET_PID, 0, 0, 0);
}

void Exit(i32 exitCode) {
    SystemCall(SYSCALL_EXIT, (u32)exitCode, 0, 0);
}

PID WaitPid(PID pid, i32* exitCode) {
    while (TRUE) {
        PID rid = SystemCall(SYSCALL_WAIT_PID, (u32)pid, (u32)exitCode, 0);
        switch (rid) {
            case -2:
                Yield();
                break;
            default:
                return rid;
        }
    }
}

String PWD() {
    return (String)SystemCall(SYSCALL_PWD, 0, 0, 0);
}

void ListDir(String path, String option) {
    SystemCall(SYSCALL_LS, (u32)path, (u32)option, 0);
}

void TouchFile(String name) {
    SystemCall(SYSCALL_CREATE_FILE, (u32)name, 0, 0);
}

void WriteToFile(String content, String filename) {
    SystemCall(SYSCALL_WRITE_FILE, (u32)content, (u32)filename, 0);
}

void BrowserFile(String filename) {
    SystemCall(SYSCALL_CAT_FILE, (u32)filename, 0, 0);
}

Boolean ChangeDir(String path) {
    return (Boolean)SystemCall(SYSCALL_CD, (u32)path, 0, 0);
}

Boolean MakeDir(String path, String option) {
    return (Boolean)SystemCall(SYSCALL_MKDIR, (u32)path, (u32)option, 0);
}

void ShowProcesses() {
    SystemCall(SYSCALL_PS, 0, 0, 0);
}

void ClearScreen() {
    SystemCall(SYSCALL_CLEAR, 0, 0, 0);
}

Size RequestPathCompletions(String path, String output, Size capacity) {
    return SystemCall(SYSCALL_PATH_COMPLETIONS, (u32)path, (u32)output, capacity);
}
