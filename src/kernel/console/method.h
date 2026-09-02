#pragma once

#include "type.h"

Size ConsoleWriteWithColor(const char *buf, Size len, ConsoleColor color);
void ConsoleAlignLine();

/// @brief 初始化控制台并清屏
void InitializeConsole();
