#pragma once

#include "../common/type.h"

#define INTERRUPT_COUNT 256
#define EXCEPTION_COUNT 32
#define OUTERAL_INTERRUPT_COUNT 16

struct InterruptDescriptor {
    u16 OffsetLow;  // 中断处理函数的偏移量 0-15
    u16 Selector;   // 中断处理函数的段选择子
    u8 Reserved;    // unused
    u8 Type : 4;    // task gate/interrupt gate/trap gate -> 0x0110/0x1110/...
    u8 Segment : 1; // 0: system segment, 1: code segment, should be 0
    u8 DPL : 2;     // DPL, should be 0
    u8 Present : 1; // present in memory?
    u16 OffsetHigh; // 中断处理函数的偏移量 16-31
} StructNotAlign;
typedef struct  InterruptDescriptor InterruptDescriptor;


struct DescriptorTablePointer {
    u16 Limit; // 表界限
    u32 Base;  // 表基址
} StructNotAlign;
typedef struct DescriptorTablePointer DescriptorTablePointer;

// 系统调用号
#define SYSCALL_TEST 0
#define SYSCALL_WRITE 1
#define SYSCALL_FORK 88
#define SYSCALL_YIELD 99
#define SYSCALL_GET_PID 111
#define SYSCALL_EXIT 444
#define SYSCALL_WAIT_PID 555
#define SYSCALL_GET_TIME 888
#define SYSCALL_READ 999

// 时钟中断
#define ClockCounter0 0x40  // 计数器0
#define ClockCounter1 0x41  // 计数器1
#define ClockCounter2 0x42  // 计数器2
#define ControlWordRegister 0x43 // 控制字寄存器
#define OsclilatorFrequency 1193180 // 振荡器振荡频率
#define ClockFrequency 100 // 时钟频率，每秒时钟打点数
#define ClockMaxCounter 11931 // 时钟倒计时初值
#define JeffyMS 10 // 时间片毫秒数，每10ms进行一次进程切换
#define MSPerTick (1.0 / OsclilatorFrequency) // 每一次时钟打点占用的秒数

// 键盘中断
#define KeyboardDataPort 0x60 // 键盘数据寄存器端口