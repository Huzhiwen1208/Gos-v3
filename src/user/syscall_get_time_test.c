#include "stdlib.h"

void syscall_get_time_test1() {
    while (1) {
        u32 ms = GetTime();
        printf("[syscall_get_time_test1] get time, current ms: %d\n", ms);
    }
}

void syscall_get_time_test2() {
    while (1) {
        u32 ms = GetTime();
        printf("[syscall_get_time_test2] get time, current ms: %d\n", ms);
    }
}

void syscall_get_time_test3() {
    while (1) {
        u32 ms = GetTime();
        printf("[syscall_get_time_test3] get time, current ms: %d\n", ms);
    }
}
