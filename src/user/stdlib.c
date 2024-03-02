#include "stdlib.h"

// TODO 实现系统调用printf
void printf(const char *fmt, ...) {
    // 1. 把这个可变字符串变成一个可打印的字符串
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    int i = Vsprintf(buf, fmt, args);
    va_end(args);
    // 2. 调用系统调用
    SyscallPrint(buf, strlen(buf));
}

int strlen(const char* str) {
    int len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}