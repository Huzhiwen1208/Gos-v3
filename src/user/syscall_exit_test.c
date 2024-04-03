#include "stdlib.h"

void syscall_exit_test1() {
    printf("I'm syscall_exit_test1 process! If the test passed, i will dead\n");
    Exit(0);
}

void syscall_exit_test2() {
    while (1) {
        for (int i = 0; i < 100000000; i++); // 类似于sleep作用
        printf("This is syscall_exit_test2 process!\n");
    }
}