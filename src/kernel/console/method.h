#pragma once

#include "type.h"

Size ConsoleWriteWithColor(const char* buf, Size len, ConsoleColor color);
void ConsoleAlignLine();
/// @brief Clear all VGA text memory and move the cursor to the top left.
void ConsoleClear();

/// @brief clear the screen and initialize
void InitializeConsole();
