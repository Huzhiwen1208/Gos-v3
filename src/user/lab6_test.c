#include "stdlib.h"

void TestSyscallGetTaskInfo();      // 系统调用GetTaskInfo测试
void TestSyscallMmap();             // 系统调用Mmap测试
void TestSyscallMunmap();           // 系统调用Munmap测试
void TestSyscallSleep();            // 系统调用Sleep测试

void TestSyscallGetTaskInfo() {
    TaskInfo info;
    GetTaskInfo(&info);
    printf("[TestSyscallGetTaskInfo] pid: %d, status: %d, time: %d\n", info.pid, info.status, info.time);
    for (int i = 0; i < 10; i++) {
        printf("[TestSyscallGetTaskInfo] syscall [%d] invoked num: %d\n", i, info.call[i]);
    }
}

void TestSyscallMmap() {
    u32 start = 0x100000;
    u32 len = 0x1000;
    u32 prot = 0x7;
    i32 ret = MMap(start, len, prot);
    printf("[TestSyscallMmap] start: 0x%x, len: 0x%x, prot: 0x%x, ret: %d\n", start, len, prot, ret);
}

void TestSyscallMunmap() {
    u32 start = 0x100000;
    u32 len = 0x1000;
    i32 ret = MUnmap(start, len);
    printf("[TestSyscallMunmap] start: 0x%x, len: 0x%x, ret: %d\n", start, len, ret);
}

void TestSyscallSleep() {
    u32 time = 1000;
    Sleep(time);
    printf("[TestSyscallSleep] sleep %d ms\n", time);
}