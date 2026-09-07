#include "common/mod.h"
#include "lib/mod.h"
#include "console/mod.h"
#include "console/type.h"
#include "memory/mod.h"
#include "int/mod.h"
#include "process/mod.h"
#include "disk/mod.h"
#include "device/mod.h"

// 进程列表
void ProcessA();
void ProcessB();
void ProcessC();
extern void user_process();
extern void syscall_get_time_test1();
extern void syscall_get_time_test2();
extern void syscall_get_time_test3();
extern void syscall_read_test();
extern void syscall_get_pid_test1();
extern void syscall_get_pid_test2();
extern void syscall_get_pid_test3();
extern void syscall_exit_test1();
extern void syscall_exit_test2();
extern void syscall_wait_pid_test();

// 测试方法列表
void TestKernelProcessWithPaging();  // 分页开启后的内核级进程调度测试
void TestUserProcessWithPageing();  // 分页开启后的用户级进程调度测试
void TestClockInterrupt();          // 时钟中断测试
void TestSyscallGetTime();          // 系统调用Read测试
void TestSyscallRead();             // 系统调用Read测试
void TestSyscallGetPid();           // 系统调用GetPid测试
void TestSyscallExit();             // 系统调用Exit测试
void TestSyscallWaitPid();          // 系统调用WaitPid测试

void TestReadWriteDisk();           // 读写磁盘测试
void TestReadWriteDiskWithCache();  // 读写磁盘测试（带缓存）

// 测试套件，主测试方法
void KernelMainTest() {
    // TestKernelProcessWithPaging();
    // TestUserProcessWithPageing();
    // TestClockInterrupt();
    // TestSyscallGetTime();
    // TestSyscallRead();
    // TestSyscallGetPid();
    // TestSyscallExit();
    // TestSyscallWaitPid();
    // TestReadWriteDisk();
    TestReadWriteDiskWithCache();
}


// 测试方法具体实现
void TestKernelProcessWithPaging() {
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

void TestSyscallGetTime() {
    CreateUserProcess(syscall_get_time_test1);
    CreateUserProcess(syscall_get_time_test2);
    CreateUserProcess(syscall_get_time_test3);
}

void TestSyscallRead() {
    CreateUserProcess(syscall_read_test);
}

void TestSyscallGetPid() {
    CreateUserProcess(syscall_get_pid_test1);
    CreateUserProcess(syscall_get_pid_test2);
    CreateUserProcess(syscall_get_pid_test3);
}

void TestSyscallExit() {
    CreateUserProcess(syscall_exit_test1);
    CreateUserProcess(syscall_exit_test2);
}

void TestSyscallWaitPid() {
    CreateUserProcess(syscall_wait_pid_test);
}

void TestReadWriteDisk() {
    // 读写磁盘测试
    char *buffer = (char *)Malloc(512);
    int i;
    for (i = 0; i < 512; i++) {
        buffer[i] = 8;
    }
    DeviceWrite(0, 0, 1, buffer);
    DeviceRead(0, 0, 1, buffer);
    for (i = 0; i < 512; i++) {
        if (buffer[i] != 8) {
            PrintWithColor(RED, "Read/Write Disk Error!\n");
            break;
        }
    }
    if (i == 512) {
        PrintWithColor(GREEN, "Read/Write Disk Success!\n");
    }
}

void TestReadWriteDiskWithCache() {
    // 读写磁盘测试（带缓存）
    char *buffer = (char *)Malloc(512);
    int i;
    for (i = 0; i < 512; i++) {
        buffer[i] = 8;
    }
    DiskCacheWrite(0, buffer);
    Free(buffer);

    char *buffer2 = (char *)Malloc(512);
    MemorySet(buffer2, 0, 512);
    DiskCacheRead(0, buffer2);

    for (i = 0; i < 512; i++) {
        if (buffer2[i] != 8) {
            PrintWithColor(RED, "Read/Write Disk With Cache Error!\n");
            break;
        }
    }

    if (i == 512) {
        PrintWithColor(GREEN, "Read/Write Disk With Cache Success!\n");
    }
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
