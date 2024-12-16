#pragma once

#include "mod.h"

/// @brief 读取端口 port 的一个字节
/// @param port 
/// @return 
u8 ReadByte(u16 port);
/// @brief 将一个字节 data 写入端口 port
/// @param port 
/// @param data 
void WriteByte(u16 port, u8 data);

void MemoryCopy(void *dest, const void *src, Size size);
void MemoryFree(void *ptr, Size size);
void MemorySet(void *ptr, u8 value, Size size);

Size StringLength(const char *str);
Boolean StringEqual(const char *str1, const char *str2);
Boolean StringStartWith(const String str1, const String start);
u32 StringToInt32(const char *str);
u32 StringASCIITotal(const char *str);
u32 CharCount(const String str, char ch);
Boolean IsDigit(char c);
Boolean IsAlpha(char c);
Boolean IsUpper(char c);
Boolean IsLower(char c);