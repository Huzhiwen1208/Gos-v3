#include "common/mod.h"
#include "console/mod.h"
#include "console/type.h"
#include "device/mod.h"
#include "disk/mod.h"
#include "fs/mod.h"
#include "int/mod.h"
#include "lib/mod.h"
#include "memory/mod.h"
#include "process/mod.h"

#define TEST_DISK_ID 0
#define TEST_DISK_BLOCK 128
#define TEST_SECTOR_SIZE 512

static u32 passedCount;
static u32 failedCount;

static void beginTest(const char *name);
static Boolean expect(Boolean condition, const char *name);
static void endSuite();
static void fillPattern(u8 *buffer, u8 seed);
static Boolean equalBuffer(const u8 *left, const u8 *right, Size size);

static void testMemoryAllocator();
static void testDiskReadWriteRestore();
static void testFileSystemCrud();
static void testPathCompletion();

/* 调度测试：这些测试会把控制权交给调度器，不能放进默认同步测试套件。 */
void TestKernelProcessWithPaging();
void TestUserProcessWithPageing();
void TestClockInterrupt();
void TestSyscallGetTime();
void TestSyscallRead();
void TestSyscallGetPid();
void TestSyscallExit();
void TestSyscallWaitPid();

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

void KernelMainTest() {
    passedCount = 0;
    failedCount = 0;
    PrintWithColor(CYAN, "\n========== Kernel test suite ==========" "\n");

    testMemoryAllocator();
    testDiskReadWriteRestore();
    testFileSystemCrud();
    testPathCompletion();

    endSuite();
}

static void testMemoryAllocator() {
    beginTest("memory allocator");
    PhysicalAddress first = Malloc(64);
    PhysicalAddress second = Malloc(512);

    expect(first != NULL, "allocates a small block");
    expect(second != NULL, "allocates a medium block");
    expect(first != second, "keeps allocations distinct");

    Free(first);
    PhysicalAddress reused = Malloc(64);
    expect(reused == first, "reuses a released block");

    Free(reused);
    Free(second);
}

/* 在空闲区域验证块设备 I/O，并无条件恢复写入前的数据。 */
static void testDiskReadWriteRestore() {
    beginTest("disk read/write and restore");
    u8 *original = (u8 *)Malloc(TEST_SECTOR_SIZE);
    u8 *pattern = (u8 *)Malloc(TEST_SECTOR_SIZE);
    u8 *readback = (u8 *)Malloc(TEST_SECTOR_SIZE);

    expect(original != NULL && pattern != NULL && readback != NULL, "allocates I/O buffers");
    if (original == NULL || pattern == NULL || readback == NULL) {
        if (original != NULL) Free((PhysicalAddress)original);
        if (pattern != NULL) Free((PhysicalAddress)pattern);
        if (readback != NULL) Free((PhysicalAddress)readback);
        return;
    }

    if (!expect(DeviceRead(TEST_DISK_ID, TEST_DISK_BLOCK, 1, original) == TEST_SECTOR_SIZE,
                "reads original sector")) {
        Free((PhysicalAddress)original);
        Free((PhysicalAddress)pattern);
        Free((PhysicalAddress)readback);
        return;
    }

    fillPattern(pattern, 0x5a);
    expect(DeviceWrite(TEST_DISK_ID, TEST_DISK_BLOCK, 1, pattern) == TEST_SECTOR_SIZE,
           "writes test pattern");
    expect(DeviceRead(TEST_DISK_ID, TEST_DISK_BLOCK, 1, readback) == TEST_SECTOR_SIZE,
           "reads written pattern");
    expect(equalBuffer(pattern, readback, TEST_SECTOR_SIZE), "preserves sector contents");
    expect(DeviceWrite(TEST_DISK_ID, TEST_DISK_BLOCK, 1, original) == TEST_SECTOR_SIZE,
           "restores original sector");

    Free((PhysicalAddress)original);
    Free((PhysicalAddress)pattern);
    Free((PhysicalAddress)readback);
}

static void testFileSystemCrud() {
    const String directory = "/kernel_test";
    const String file = "/kernel_test/data.txt";
    const String filename = "data.txt";
    const String content = "kernel filesystem test\n.";
    const String firstLine = "kernel filesystem test";
    char line[64];
    Boolean createdDirectory;
    Boolean enteredDirectory;

    beginTest("filesystem CRUD");
    RemoveFile("-r", directory);  // 清除前次测试残留；不存在时允许失败。
    createdDirectory = MakeDirectory(directory, "-p");
    expect(createdDirectory, "creates test directory");
    if (!createdDirectory) return;

    enteredDirectory = ChangeDirectory(directory);
    expect(enteredDirectory, "enters test directory");
    if (!enteredDirectory) {
        RemoveFile("-r", directory);
        return;
    }

    expect(CreateFile(-1, filename, FT_FILE) != (InodeID)-1, "creates test file");
    expect(WriteFileContent(file, content, TRUE), "writes test file");
    expect(ReadFileLine(file, 1, line), "reads first line");
    expect(StringEqual(line, firstLine), "reads back expected content");
    expect(ChangeDirectory("/"), "returns to root directory");
    expect(RemoveFile("-r", directory), "removes test directory");
}

static void testPathCompletion() {
    const String directory = "/kernel_completion_test";
    char completions[128];

    beginTest("filesystem path completion");
    RemoveFile("-r", directory);
    expect(MakeDirectory(directory, "-p"), "creates completion test directory");
    if (!ChangeDirectory(directory)) {
        expect(FALSE, "enters completion test directory");
        return;
    }

    expect(CreateFile(-1, "alpha", FT_FILE) != (InodeID)-1, "creates first matching file");
    expect(MakeDirectory("alpine", ""), "creates matching directory");
    expect(GetPathCompletions("al", completions, sizeof(completions)) == 2,
           "finds matching path candidates");
    expect(StringEqual(completions, "alpha\nalpine/\n"), "marks directories with slash");
    expect(GetPathCompletions("alpha", completions, sizeof(completions)) == 1,
           "finds exact path candidate");
    expect(ChangeDirectory("/"), "returns to root after completion test");
    expect(RemoveFile("-r", directory), "removes completion test directory");
}

static void beginTest(const char *name) {
    PrintWithColor(LIGHT_BLUE, "[TEST] %s\n", name);
}

static Boolean expect(Boolean condition, const char *name) {
    if (condition) {
        passedCount++;
        PrintWithColor(GREEN, "  PASS: %s\n", name);
    } else {
        failedCount++;
        PrintWithColor(LIGHT_RED, "  FAIL: %s\n", name);
    }
    return condition;
}

static void endSuite() {
    ConsoleAlignLine();
    if (failedCount == 0) {
        PrintWithColor(GREEN, "Kernel tests passed: %d assertions\n", passedCount);
    } else {
        PrintWithColor(LIGHT_RED, "Kernel tests failed: %d passed, %d failed\n", passedCount, failedCount);
    }
}

static void fillPattern(u8 *buffer, u8 seed) {
    for (Size index = 0; index < TEST_SECTOR_SIZE; index++) {
        buffer[index] = seed + index;
    }
}

static Boolean equalBuffer(const u8 *left, const u8 *right, Size size) {
    for (Size index = 0; index < size; index++) {
        if (left[index] != right[index]) return FALSE;
    }
    return TRUE;
}

void TestKernelProcessWithPaging() {
    extern void ProcessA();
    extern void ProcessB();
    extern void ProcessC();
    CreateKernelProcess(ProcessA);
    CreateKernelProcess(ProcessB);
    CreateKernelProcess(ProcessC);
    Schedule();
}

void TestUserProcessWithPageing() { CreateUserProcess(user_process); Schedule(); }
void TestClockInterrupt() { CreateKernelProcess(TestKernelProcessWithPaging); }
void TestSyscallGetTime() { CreateUserProcess(syscall_get_time_test1); CreateUserProcess(syscall_get_time_test2); CreateUserProcess(syscall_get_time_test3); }
void TestSyscallRead() { CreateUserProcess(syscall_read_test); }
void TestSyscallGetPid() { CreateUserProcess(syscall_get_pid_test1); CreateUserProcess(syscall_get_pid_test2); CreateUserProcess(syscall_get_pid_test3); }
void TestSyscallExit() { CreateUserProcess(syscall_exit_test1); CreateUserProcess(syscall_exit_test2); }
void TestSyscallWaitPid() { CreateUserProcess(syscall_wait_pid_test); }

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
