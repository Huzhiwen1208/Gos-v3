#pragma once

#include "type.h"

void ConsoleAlignLine();
Size ConsoleWriteWithColor(const char *buf, Size len, ConsoleColor color);

/// @brief clear the screen and initialize
void InitializeConsole();