#!/usr/bin/env python3
"""Run FS regressions on a poisoned in-memory disk; requires macOS clang (or CC).

The temporary host build widens PhysicalAddress only, so the kernel's 32-bit
allocator interface can use host pointers. DiskInode's 128-byte layout stays
unchanged and is checked by the production _Static_assert declarations.
No disk images, kernel build products, or source files are modified.
Set FS_TEST_SANITIZE=1 to also enable AddressSanitizer and UndefinedBehaviorSanitizer.
"""
from pathlib import Path
import os
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
HARNESS = r'''
#include "src/kernel/fs/fs.c"
#undef Free
extern void* malloc(unsigned long);
extern void free(void*);
extern int printf(const char*, ...);
extern int vsnprintf(char*, unsigned long, const char*, __builtin_va_list);
extern void exit(int);

#define SECTORS 32768
static u8 disk[SECTORS][512];
static char output[131072];
static u32 outputLength;
static u32 checks;
static IdeDisk ideDisk;
static Device device;
static ReentrantLock lock;

static void check(Boolean value, const char* expression, u32 line) {
    checks++;
    if (!value) { printf("FAIL line %u: %s\nOutput: %s\n", line, expression, output); exit(1); }
}
#define CHECK(value) check(!!(value), #value, __LINE__)
static void resetOutput() { outputLength = 0; output[0] = 0; }
static Boolean contains(const char* text) {
    for (u32 i = 0; output[i]; i++) {
        u32 j = 0;
        while (text[j] && output[i + j] == text[j]) j++;
        if (!text[j]) return TRUE;
    }
    return FALSE;
}
static Size capture(const char* format, __builtin_va_list args) {
    int count = vsnprintf(output + outputLength, sizeof(output) - outputLength, format, args);
    if (count < 0 || outputLength + (u32)count >= sizeof(output)) exit(2);
    outputLength += count;
    return count;
}
Size Printf(const char* format, ...) {
    __builtin_va_list args; __builtin_va_start(args, format);
    Size count = capture(format, args); __builtin_va_end(args); return count;
}
Size PrintWithColor(ConsoleColor color, const char* format, ...) {
    if (color == LIGHT_BLUE) Printf("<blue>");
    __builtin_va_list args; __builtin_va_start(args, format);
    Size count = capture(format, args); __builtin_va_end(args); return count;
}
void Panic(const char* format, ...) { printf("Unexpected kernel panic: %s\n", format); exit(2); }
PhysicalAddress Malloc(Size size) { void* result = malloc(size); if (!result) exit(2); return (PhysicalAddress)result; }
void Free(PhysicalAddress address) { free((void*)address); }
void MemoryCopy(void* destination, const void* source, Size size) {
    u8* d = destination; const u8* s = source; for (u32 i = 0; i < size; i++) d[i] = s[i];
}
void MemorySet(void* address, u8 value, Size size) { u8* p = address; for (u32 i = 0; i < size; i++) p[i] = value; }
void MemoryFree(void* address, Size size) { MemorySet(address, 0, size); }
Size StringLength(const char* text) { Size length = 0; while (text[length]) length++; return length; }
Boolean StringEqual(const char* a, const char* b) { while (*a && *a == *b) { a++; b++; } return *a == *b; }
static void noopLock(ReentrantLock* ignored) { (void)ignored; }
ReentrantLock* NewReentrantLock() { lock.Lock = noopLock; lock.Unlock = noopLock; return &lock; }
Device* GetDeviceByID(u32 id) { CHECK(id == 1); return &device; }
void DiskCacheRead(u32 block, void* buffer) {
    if (block >= SECTORS) { printf("Invalid block read: %u\n", block); exit(2); }
    MemoryCopy(buffer, disk[block], 512);
}
void DiskCacheWrite(u32 block, void* buffer) {
    if (block >= SECTORS) { printf("Invalid block write: %u\n", block); exit(2); }
    MemoryCopy(disk[block], buffer, 512);
}

int main(void) {
    MemorySet(disk, 0xa5, sizeof(disk));
    ideDisk.TotalSector = SECTORS;
    device.DevicePtr = &ideDisk;
    InitializeFileSystem();
    CHECK(sizeof(DiskInode) == 128);
    CHECK(StringEqual(GetCurrentPath(), "/"));
    CHECK(ChangeDirectory("~")); CHECK(StringEqual(GetCurrentPath(), "/home/JackHu"));
    CHECK(ChangeDirectory("/home//JackHu/../JackHu/./"));
    CHECK(StringEqual(GetCurrentPath(), "/home/JackHu"));
    CHECK(!ChangeDirectory("missing/../JackHu"));
    CHECK(StringEqual(GetCurrentPath(), "/home/JackHu"));
    resetOutput(); CHECK(!MakeDirectory("", "")); CHECK(lastPathError == PATH_INVALID); CHECK(contains("Invalid path"));
    CHECK(!MakeDirectory(".", "")); CHECK(MakeDirectory(".", "-p"));
    CHECK(!MakeDirectory("missing/child", "")); CHECK(pathToInodeID("missing") == INVALID_INODE);
    CHECK(MakeDirectory("/a/b/c", "-p")); CHECK(!MakeDirectory("/a/b/c", ""));
    CHECK(MakeDirectory("/a/b/c", "-p")); CHECK(ChangeDirectory("/a/b"));
    CHECK(MakeDirectory("../sibling", "")); CHECK(pathToInodeID("/a/sibling") != INVALID_INODE);
    InodeID plain = CreateFile(INVALID_INODE, "plain", FT_FILE);
    CHECK(plain != INVALID_INODE);
    CHECK(!ChangeDirectory("plain/../c")); CHECK(lastPathError == PATH_NOT_DIRECTORY);
    CHECK(StringEqual(GetCurrentPath(), "/a/b"));
    CHECK(!MakeDirectory("plain/child", "-p"));
    CHECK(CreateFile(INVALID_INODE, "", FT_FILE) == INVALID_INODE);
    CHECK(CreateFile(INVALID_INODE, "..", FT_FILE) == INVALID_INODE);
    CHECK(CreateFile(INVALID_INODE, "a/b", FT_FILE) == INVALID_INODE);
    char name[FS_NAME_MAX + 1]; MemorySet(name, 'n', sizeof(name)); name[27] = 0;
    CHECK(CreateFile(INVALID_INODE, name, FT_DIRECTORY) != INVALID_INODE);
    CHECK(ChangeDirectory(name)); CHECK(ChangeDirectory(".."));
    name[27] = 'n'; name[28] = 0;
    CHECK(CreateFile(INVALID_INODE, name, FT_FILE) == INVALID_INODE);
    resetOutput(); CHECK(!MakeDirectory(name, "")); CHECK(lastPathError == PATH_NAME_TOO_LONG); CHECK(contains("File name too long"));
    char tooLong[FS_PATH_MAX + 1]; MemorySet(tooLong, 'q', sizeof(tooLong)); tooLong[256] = 0;
    resetOutput(); CHECK(!ChangeDirectory(tooLong)); CHECK(lastPathError == PATH_TOO_LONG); CHECK(contains("Path too long"));
    CHECK(StringEqual(GetCurrentPath(), "/a/b"));
    char longest[FS_PATH_MAX]; u32 used = 0;
    longest[used++] = '/'; longest[used++] = 'z'; longest[used++] = 'z';
    for (u32 component = 0; component < 9; component++) {
        longest[used++] = '/';
        for (u32 i = 0; i < 27; i++) longest[used++] = 'v';
    }
    longest[used] = 0;
    CHECK(used == FS_PATH_MAX - 1); CHECK(MakeDirectory(longest, "-p"));
    CHECK(ChangeDirectory(longest)); CHECK(!MakeDirectory("x", "")); CHECK(lastPathError == PATH_TOO_LONG);
    CHECK(StringEqual(GetCurrentPath(), longest)); CHECK(ChangeDirectory("/"));

    CHECK(MakeDirectory("/private", "")); CHECK(ChangeDirectory("/private"));
    CHECK(SetFilePermissions("/private", 0000));
    CHECK(!ChangeDirectory("..")); CHECK(lastPathError == PATH_PERMISSION);
    CHECK(StringEqual(GetCurrentPath(), "/private"));
    CHECK(ChangeDirectory("/")); CHECK(!ChangeDirectory("/private"));
    resetOutput(); ListFiles("", "/private"); CHECK(contains("Permission denied"));
    CHECK(!MakeDirectory("/private/new", "")); CHECK(lastPathError == PATH_PERMISSION);
    CHECK(SetFilePermissions("/private", 0300));
    CHECK(ChangeDirectory("/private"));
    resetOutput(); ListFiles("", "."); CHECK(contains("Permission denied"));
    CHECK(ChangeDirectory("/")); CHECK(SetFilePermissions("/private", 0500));
    CHECK(!MakeDirectory("/private/new", "")); CHECK(lastPathError == PATH_PERMISSION);
    CHECK(SetFilePermissions("/private", 0755));
    CHECK(CreateFile(pathToInodeID("/private"), "sample", FT_FILE) != INVALID_INODE);
    CHECK(SetFilePermissions("/private", 0500));
    CHECK(!WriteFileContent("/private/missing", "data", TRUE));
    CHECK(SetFilePermissions("/private", 0755));
    CHECK(ChangeDirectory("/private"));
    CHECK(WriteFileContent("sample", "before\n", TRUE));
    CHECK(SetFilePermissions("sample", 0400));
    CHECK(!WriteFileContent("sample", "after", TRUE));
    resetOutput(); CHECK(PrintFileContent("sample", (u32)-1)); CHECK(StringEqual(output, "before\n"));
    CHECK(SetFilePermissions("sample", 0200));
    resetOutput(); CHECK(!PrintFileContent("sample", (u32)-1)); CHECK(contains("Permission denied"));
    CHECK(SetFilePermissions("sample", 0644));
    char payload[1101]; MemorySet(payload, 'x', 1100); payload[1100] = 0;
    CHECK(WriteFileContent("./sample", payload, TRUE));
    CHECK(WriteFileContent("/private/../private/sample", "tail", FALSE));
    resetOutput(); CHECK(PrintFileContent("sample", (u32)-1)); CHECK(outputLength == 1104);
    CHECK(StringEqual(output + 1100, "tail"));
    char line[1200]; CHECK(ReadFileLine("sample", 1, line)); CHECK(StringLength(line) == 1104);

    CHECK(MakeDirectory("/listing", "")); CHECK(ChangeDirectory("/listing"));
    for (u32 i = 0; i < 20; i++) {
        char fileName[4] = {'f', (char)('0' + i / 10), (char)('0' + i % 10), 0};
        CHECK(CreateFile(INVALID_INODE, fileName, FT_FILE) != INVALID_INODE);
    }
    CHECK(MakeDirectory("sub", "")); CHECK(CreateFile(INVALID_INODE, ".hidden", FT_FILE) != INVALID_INODE);
    DiskInode* listing = readDiskInode(currentDirectoryInodeID);
    u32 position = listing->Size;
    DirectoryEntry* ghost = (DirectoryEntry*)(disk[listing->DirectBlock[position / 512]] + position % 512);
    ghost->InodeID = pathToInodeID("sub"); MemoryCopy(ghost->Name, "ghost", 6); Free((PhysicalAddress)listing);
    resetOutput(); ListFiles("", "."); CHECK(contains("f00")); CHECK(contains("f19")); CHECK(contains("<blue>sub/"));
    CHECK(!contains(".hidden")); CHECK(!contains("ghost"));
    CHECK(!ChangeDirectory("ghost")); CHECK(StringEqual(GetCurrentPath(), "/listing"));
    resetOutput(); ListFiles("-a", "../listing/."); CHECK(contains("./\t../")); CHECK(contains(".hidden"));
    resetOutput(); ListFiles("", "f19"); CHECK(StringEqual(output, "f19\n"));
    CHECK(WriteFileContent("f00", "readonly", TRUE));
    CHECK(SetFilePermissions("f00", 0400));
    CHECK(RemoveFile("", "f00"));
    resetOutput(); ListFiles("", "."); CHECK(!contains("f00")); CHECK(contains("f19")); CHECK(contains("sub/"));
    CHECK(ChangeDirectory("/")); CHECK(SetFilePermissions("/private", 0700));
    InitializeFileSystem(); CHECK(StringEqual(GetCurrentPath(), "/"));
    CHECK(pathToInodeID("/a/b/c") != INVALID_INODE);
    DiskInode* preserved = readDiskInode(pathToInodeID("/private")); CHECK(preserved->Permissions == 0700); Free((PhysicalAddress)preserved);
    ((SuperBlock*)disk[0])->Magic = FS_LEGACY_MAGIC;
    InitializeFileSystem();
    preserved = readDiskInode(pathToInodeID("/private")); CHECK(preserved->Permissions == 0755); Free((PhysicalAddress)preserved);
    resetOutput(); CHECK(PrintFileContent("/private/sample", (u32)-1)); CHECK(outputLength == 1104);
    printf("PASS: %u filesystem checks (paths, permissions, directory size, file IO, remount, migration)\n", checks);
    return 0;
}
'''


def main():
    with tempfile.TemporaryDirectory(prefix="gos-fs-test-") as directory:
        temporary = Path(directory)
        shutil.copytree(ROOT / "src/kernel", temporary / "src/kernel")
        types = temporary / "src/kernel/common/type.h"
        types.write_text(types.read_text().replace("typedef unsigned int PhysicalAddress;", "typedef unsigned long PhysicalAddress;"))
        source = temporary / "fs_memory.c"
        source.write_text(HARNESS)
        binary = temporary / "fs_memory"
        flags = ["-std=gnu11", "-Wall", "-Wextra", "-Werror", "-fno-builtin", "-g"]
        if os.environ.get("FS_TEST_SANITIZE"):
            flags += ["-fsanitize=address,undefined", "-fno-omit-frame-pointer"]
        subprocess.run([os.environ.get("CC", "clang"), *flags, str(source), "-o", str(binary)], check=True)
        subprocess.run([str(binary)], check=True)


if __name__ == "__main__":
    main()
