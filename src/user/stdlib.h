#include "../kernel/user_lib/mod.h"
#include "../kernel/lib/method.h"

void printf(const char *fmt, ...);
int strlen(const char* str);
int strcmp(const char* str1, const char* str2);
int memset(void* ptr, int value, int num);
int strcpy(char* dst, const char* src, int size);
void shell();
