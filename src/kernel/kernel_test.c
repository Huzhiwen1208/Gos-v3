#include "common/mod.h"
#include "lib/mod.h"
#include "console/mod.h"
#include "memory/mod.h"
#include "int/mod.h"
#include "process/mod.h"

// 进程列表
void ProcessA();
void ProcessB();
void ProcessC();
extern void user_process();
extern void syscall_read_test();
extern void syscall_get_pid_test1();
extern void syscall_get_pid_test2();
extern void syscall_get_pid_test3();

// 测试方法列表
void TestKernelProcessWithPaging();  // 分页开启后的内核级进程调度测试
void TestUserProcessWithPageing();  // 分页开启后的用户级进程调度测试
void TestClockInterrupt();          // 时钟中断测试
void TestSyscallRead();             // 系统调用Read测试
void TestSyscallGetPid();           // 系统调用GetPid测试

// 测试套件，主测试方法
void KernelMainTest() {
    // TestKernelProcessWithPaging();
    // TestUserProcessWithPageing();
    // TestClockInterrupt();
    // TestSyscallRead();
    TestSyscallGetPid();
}


// 测试方法具体实现
void TestKernelProcessWithPaging(){
    CreateKernelProcess(ProcessA);
    CreateKernelProcess(ProcessB);
    CreateKernelProcess(ProcessC);
    Schedule();
}

void TestUserProcessWithPageing() {
    CreateUserProcess(user_process);
    Schedule();
}

void TestClockInterrupt() {
    CreateKernelProcess(ProcessA);
    CreateKernelProcess(ProcessB);
    CreateKernelProcess(ProcessC);
}

void TestSyscallRead() {
    CreateUserProcess(syscall_read_test);
}

void TestSyscallGetPid() {
    CreateUserProcess(syscall_get_pid_test1);
    CreateUserProcess(syscall_get_pid_test2);
    CreateUserProcess(syscall_get_pid_test3);
}


void ProcessA() {
    while (TRUE) {
        PrintWithColor(GREEN, "This is A process\n");
        Schedule();
    }
}

void ProcessB() {
    while (TRUE) {
        PrintWithColor(YELLOW, "This is B process\n");
        Schedule();
    }
}

void ProcessC() {
    while (TRUE) {
        PrintWithColor(BLUE, "This is C process\n");
        Schedule();
    }
}