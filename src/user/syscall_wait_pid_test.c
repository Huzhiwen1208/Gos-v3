#include "stdlib.h"

void syscall_wait_pid_test() {
    printf("[syscall_wait_pid_test] entry\n");
    PID pid = Fork();
    if (pid) {
        // 父进程
        PID id = GetPID();
        i32 exitCode;
        printf("This is father! My pid: %d, I will wait my son: {son's pid: %d}\n", id, pid);
        PID cid = WaitPid(pid, &exitCode);
        printf("This is father! My pid: %d, I have killed my son: {son's pid: %d}\n", id, pid);
    } else {
        // 子进程
        Yield();
        PID pid = GetPID();
        printf("This is son! My pid: %d, I will be exited\n", pid);
        Exit(-1);
    }
    while(1);
}