#include "stdlib.h"

void user_process() {
    int pid = Fork();
    if (pid == 0) {
        // 子进程
        while (1) {
            int a = 3;
            a += 2;
            Yield();
        }
    }else {
        // 父进程
        while (1) {
            int b = 3;
            b += 3;
            Yield();
        }
    }
}