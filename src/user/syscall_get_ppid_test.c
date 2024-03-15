#include "stdlib.h"

void syscall_get_ppid_test() {
    int pid = Fork();
    if (pid) {
        // 父进程
        int ppid = GetPPID();
        int pid = GetPID();
        printf("Parent:  my id: %d, pid: %d\n", pid, ppid);
    }else {
        int ppid = GetPPID();
        int pid = GetPID();
        printf("Child:   my id: %d, pid: %d\n", pid, ppid);
    }

    while(1);
}