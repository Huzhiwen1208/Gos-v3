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
    Exit(0);
}

void TestSyscallMmap() {
    u32 start = 0x100000;
    u32 len = 4096 * 2; // 2个页
    u32 prot = 0x7;
    i32 ret = MMap(start, len, prot);
    printf("[TestSyscallMmap] start: 0x%x, len: 0x%x, prot: 0x%x, ret: %d\n", start, len, prot, ret);

    char* ptr = (char*)start;
    for (int i = 0; i < len; i++) {
        ptr[i] = 'A';
    }
    printf("[TestSyscallMmap] write 'A' to 0x%x\n", start);

    for (int i = 0; i < len; i++) {
        printf("%c", ptr[i]);
    }
    Exit(0);
}

void TestSyscallMunmap() {
    u32 start = 0x100000;
    u32 len = 4096 * 2; // 2个页
    u32 prot = 0x7;
    i32 ret = MMap(start, len, prot);
    printf("[TestSyscallMunmap] start: 0x%x, len: 0x%x, prot: 0x%x, ret: %d\n", start, len, prot, ret);

    char* ptr = (char*)start;
    for (int i = 0; i < len; i++) {
        ptr[i] = 'A';
    }
    printf("[TestSyscallMunmap] write 'A' to 0x%x\n", start);

    for (int i = 0; i < len; i++) {
        printf("%c", ptr[i]);
    }

    ret = MUnmap(start, len);
    printf("[TestSyscallMunmap] start: 0x%x, len: 0x%x, ret: %d\n", start, len, ret);
    Exit(0);
}

void TestSyscallSleep() {
    u32 time = 1000; // ms
    printf("[TestSyscallSleep] sleep start %d ms\n", time);
    Sleep(time);
    printf("[TestSyscallSleep] sleep end %d ms\n", time);
    Exit(0);
}

void idle_user_task() {
    while (TRUE) {
        Yield();
    }
}