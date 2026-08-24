#include "stdlib.h"

void user_process() {
    while (1) {
        printf("xxx\n");
        int pid = Fork();
        if (pid == 0) {
            shell();
        }else {
            // 父进程
            i32 exitCode;
            WaitPid(pid, &exitCode);
        }
    }
}