#include "stdlib.h"

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

int strcmp(const char* str1, const char* str2) {
    int i = 0;
    while (str1[i] != '\0' && str2[i] != '\0') {
        if (str1[i] != str2[i]) {
            return str1[i] - str2[i];
        }
        i++;
    }
    return str1[i] - str2[i];
}

int memset(void* ptr, int value, int num) {
    char* p = (char*)ptr;
    for (int i = 0; i < num; i++) {
        p[i] = value;
    }
    return num;
}

int strcpy(char* dst, const char* src, int size) {
    for (int i = 0; i < size; i++)
        dst[i] = src[i];
}