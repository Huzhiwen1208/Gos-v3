#pragma once

#include "type.h"

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
