#pragma once

void InitializeProcessManager();
PID AllocatePID();
void FreePID(PID pid);
PCB* GetCurrentProcess();
void AddProcess(PCB* process);
void Schedule();
void SwitchProcess(PCB* current, PCB* next);

void CreateKernelProcess(void* entry);
void CreateUserProcess(void* entry);

PID ForkProcess();

void RedirectParentOfChildren();
PCB* FindActivatedChildProcessByPID(PID pid);
PCB* TakeZombieProcess(PID pid);
void ExitProcess(i32 exitCode);
PID WaitProcess(PID pid, i32* exitCode);
