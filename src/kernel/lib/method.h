#pragma once

#include "../common/method.h"

Size Printf(const char *fmt, ...);
Size Println(const char *fmt, ...);
Size PrintWithColor(ConsoleColor color, const char *fmt, ...);
void Panic(const char *fmt, ...);

// Assertion
void AssertFailed(char *exp, char *file, char *base, i32 line);
#define Assert(exp) \
    if (!(exp)) AssertFailed(#exp, __FILE__, __BASE_FILE__, __LINE__)