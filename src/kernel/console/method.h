#pragma once

#include "type.h"

void ConsoleAlignLine();
Size ConsoleWriteWithColor(const char *buf, Size len, ConsoleColor color);

/// @brief 初始化控制台并清屏
void InitializeConsole();