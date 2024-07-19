#include "method.h"
#include "type.h"
#include "../common/method.h"
#include "../lib/method.h"
#include "../process/method.h"

// global variables
/// @brief 系统从开启时钟至今的时间片(10ms)数
u64 jiffies;
static u32 ClockVector = 0x20;

// static methods

static void clockInterruptHandler(u32 vector);
static void initPIT();
static u16 readCounter0();

// public methods

void InitializeClock() {
    initPIT();
    SetInterruptHandler(ClockVector, clockInterruptHandler);
    SetInterrupt(ClockVector);
    jiffies = 0;
    asm volatile("sti");
}

u32 GetTimeMS() {
    return jiffies * JeffyMS + (ClockMaxCounter - readCounter0()) * (MSPerTick * 1000);
}

// static methods implementation

static void clockInterruptHandler(u32 vector) {
    Assert(vector == ClockVector);
    jiffies++;
    OuteralInterruptCompleted(vector);
    Schedule();
}

static void initPIT() {
    WriteByte(ControlWordRegister, 0b00110100); // 00, 11, 010, 0
    WriteByte(ClockCounter0, ClockMaxCounter & 0xFF); // low byte
    WriteByte(ClockCounter0, ClockMaxCounter >> 8); // high byte
}

static u16 readCounter0() {
    WriteByte(ControlWordRegister, 0b00110100);
    u8 low = ReadByte(ClockCounter0);
    u8 high = ReadByte(ClockCounter0);
    return (high << 8) | low;
}
