#include "stdlib.h"

void syscall_get_pid_test1() {
    while (1) {
        int pid = GetPID();
        printf("[syscall_get_pid_test1] get pid: %d\n", pid);
    }
}

void syscall_get_pid_test2() {
    while (1) {
        int pid = GetPID();
        printf("[syscall_get_pid_test2] get pid: %d\n", pid);
    }
}

void syscall_get_pid_test3() {
    while (1) {
        int pid = GetPID();
        printf("[syscall_get_pid_test3] get pid: %d\n", pid);
    }
}
