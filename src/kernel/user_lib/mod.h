#pragma once

#include "../common/type.h"

void SyscallTest();
void SyscallPrint(String message, Size len);
u32 Fork();
void Yield();
Size Read(String buf, Size len);
char GetChar();
PID GetPID();
u32 GetTime();
void Exit(i32 exitCode);
PID WaitPid(PID pid, i32* exitCode);
String PWD();
void ListDir(String path, String option);
void TouchFile(String path);
void WriteToFile(String content, String filename);
void BrowserFile(String filename);
Boolean MakeDir(String path, String option);
Boolean ChangeDir(String path);
void ClearScreen();

Boolean RemoveFilePath(String path, u32 flags);
Boolean CopyFilePath(String source, String target, u32 flags);
Boolean MoveFilePath(String source, String target, u32 flags);
