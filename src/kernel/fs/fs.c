#include "method.h"
#include "type.h"
#include "../common/method.h"
#include "../device/method.h"
#include "../disk/method.h"
#include "../ds/method.h"
#include "../lib/method.h"
#include "../memory/method.h"
#include "../int/method.h"

/* The allocator is addressed by 32-bit physical addresses. */
#define Free(address) Free((PhysicalAddress)(address))

/**
 * 我们第一版先不做文件系统的一级间接和二级间接。后面会进行补充 TODO
*/

// global variables
static GFS easyFileSystem;
static InodeID currentDirectoryInodeID; // 默认为根目录
static String currentPath = NULL; // 默认为根目录

// static methods declaration

static BitMap* newBitMap(u32 startBlockID, u32 blocks);
static DiskInode* newEmptyDiskInode();
static DirectoryEntry* newDirectoryEntry(char* name, InodeID id);
static void writeDiskInode(InodeID id, DiskInode* diskInode);
static DiskInode* readDiskInode(InodeID id);
static InodeID allocateOneInodeID();
static void freeOneInodeID(InodeID id);
/// @brief 申请一个数据块
/// @return BlockID
static u32 allocateOneDataBlockID();
static void freeOneDataBlockID(u32 blockID);
/// @brief 从目录中查找文件名为name的文件的InodeID
/// @param id 目录的InodeID，不存在则返回 INVALID_INODE
static InodeID findInodeIDByNameFromDirectory(InodeID id, String name);
static Boolean isDirectory(InodeID id);
static Boolean isFile(InodeID id);
static InodeID pathToInodeID(String path);
static void pathBackStep(String path);

#define INVALID_INODE ((InodeID)-1)
#define FS_MAGIC 0x20001209
#define FS_LEGACY_MAGIC 0x20001208

typedef enum PathError {
    PATH_OK, PATH_MISSING, PATH_NOT_DIRECTORY, PATH_TOO_LONG,
    PATH_NAME_TOO_LONG, PATH_EXISTS, PATH_PERMISSION, PATH_INVALID
} PathError;
static PathError lastPathError;
static Boolean resolvePath(String path, InodeID* result, String canonical, u32 createMode);
static void printPathError(String command, String path);
static Boolean hasPermission(InodeID id, u16 permission);
static Boolean validName(String name);
static u32 boundedLength(String text, u32 limit);


// public methods

void PrintfWorkingDirectory() {
    Printf("%s\n", currentPath);
}

String GetCurrentPath() {
    return currentPath;
}

Boolean ClearFileContentByInodeID(InodeID id) {
    if (!isFile(id) || !hasPermission(id, FS_OWNER_WRITE)) return FALSE;
    DiskInode* inode = readDiskInode(id);
    u32 blocks = (inode->Size + 511) / 512;
    if (blocks > INODE_DIRECT_BLOCKS) { Free(inode); return FALSE; }
    for (u32 i = 0; i < blocks; i++) {
        freeOneDataBlockID(inode->DirectBlock[i]);
        inode->DirectBlock[i] = 0;
    }
    inode->Size = 0;
    writeDiskInode(id, inode);
    Free(inode);
    return TRUE;
}

Boolean ClearFileContent(String path) {
    InodeID id = pathToInodeID(path);
    return ClearFileContentByInodeID(id);
}

Boolean ReadFileLine(String path, u32 n, String buf) {
    InodeID id;
    char canonical[FS_PATH_MAX];
    if (!resolvePath(path, &id, canonical, 0)) { printPathError("cat", path); return FALSE; }
    if (!isFile(id)) { PrintWithColor(LIGHT_RED, "cat: %s: Is a directory\n", path); return FALSE; }
    if (!hasPermission(id, FS_OWNER_READ)) { lastPathError = PATH_PERMISSION; printPathError("cat", path); return FALSE; }
    if (!n) { buf[0] = '\0'; return TRUE; }
    DiskInode* inode = readDiskInode(id);
    u32 line = 1, used = 0;
    u8 buffer[512];
    Boolean found = FALSE;
    for (u32 position = 0; position < inode->Size; position++) {
        if (!(position % 512)) DiskCacheRead(inode->DirectBlock[position / 512], buffer);
        u8 byte = buffer[position % 512];
        if (byte == (u8)EOF) break;
        if (line == n) {
            found = TRUE;
            if (byte == '\n') break;
            buf[used++] = byte;
        }
        if (byte == '\n') line++;
    }
    buf[used] = '\0';
    Free(inode);
    return found;
}

Boolean WriteFileContent(String path, String content, Boolean trunc) {
    InodeID id;
    char canonical[FS_PATH_MAX];
    if (!resolvePath(path, &id, canonical, 0)) { printPathError("echo", path); return FALSE; }
    if (!isFile(id)) { PrintWithColor(LIGHT_RED, "echo: %s: Is a directory\n", path); return FALSE; }
    if (!hasPermission(id, FS_OWNER_WRITE)) { lastPathError = PATH_PERMISSION; printPathError("echo", path); return FALSE; }
    u32 length = boundedLength(content, INODE_DIRECT_BLOCKS * 512);
    DiskInode* inode = readDiskInode(id);
    u32 start = trunc || !inode->Size ? 0 : inode->Size - 1;
    if (length == INODE_DIRECT_BLOCKS * 512 || start + length + 1 > INODE_DIRECT_BLOCKS * 512) {
        PrintWithColor(LIGHT_RED, "echo: %s: File too large\n", path);
        Free(inode);
        return FALSE;
    }
    Free(inode);
    if (trunc && !ClearFileContentByInodeID(id)) return FALSE;
    inode = readDiskInode(id);
    u8 buffer[512];
    u32 blockIndex = (u32)-1;
    for (u32 i = 0; i <= length; i++) {
        u32 position = start + i;
        u32 nextBlock = position / 512;
        if (nextBlock != blockIndex) {
            if (blockIndex != (u32)-1) DiskCacheWrite(inode->DirectBlock[blockIndex], buffer);
            blockIndex = nextBlock;
            if (!inode->DirectBlock[blockIndex]) {
                inode->DirectBlock[blockIndex] = allocateOneDataBlockID();
                if (!inode->DirectBlock[blockIndex]) {
                    Free(inode);
                    PrintWithColor(LIGHT_RED, "echo: %s: No space left on device\n", path);
                    return FALSE;
                }
                MemorySet(buffer, 0, sizeof(buffer));
            } else DiskCacheRead(inode->DirectBlock[blockIndex], buffer);
        }
        buffer[position % 512] = i < length ? (u8)content[i] : (u8)EOF;
    }
    DiskCacheWrite(inode->DirectBlock[blockIndex], buffer);
    inode->Size = start + length + 1;
    writeDiskInode(id, inode);
    Free(inode);
    return TRUE;
}

Boolean PrintFileContent(String path, u32 n) {
    InodeID id;
    char canonical[FS_PATH_MAX];
    if (!resolvePath(path, &id, canonical, 0)) { printPathError("cat", path); return FALSE; }
    if (!isFile(id)) { PrintWithColor(LIGHT_RED, "cat: %s: Is a directory\n", path); return FALSE; }
    if (!hasPermission(id, FS_OWNER_READ)) { lastPathError = PATH_PERMISSION; printPathError("cat", path); return FALSE; }
    if (!n) { Printf("\n"); return TRUE; }
    DiskInode* inode = readDiskInode(id);
    u8 buffer[512];
    for (u32 position = 0; position < inode->Size; position++) {
        if (!(position % 512)) DiskCacheRead(inode->DirectBlock[position / 512], buffer);
        u8 byte = buffer[position % 512];
        if (byte == (u8)EOF) break;
        Printf("%c", byte);
        if (byte == '\n' && --n == 0) break;
    }
    Free(inode);
    return TRUE;
}

Boolean ChangeDirectory(String path) {
    InodeID id;
    char canonical[FS_PATH_MAX];
    if (!path || !path[0]) path = FS_HOME_PATH;
    if (!resolvePath(path, &id, canonical, 0)) {
        printPathError("cd", path);
        return FALSE;
    }
    if (!isDirectory(id)) {
        lastPathError = PATH_NOT_DIRECTORY;
        printPathError("cd", path);
        return FALSE;
    }
    if (!hasPermission(id, FS_OWNER_EXECUTE)) {
        lastPathError = PATH_PERMISSION;
        printPathError("cd", path);
        return FALSE;
    }
    currentDirectoryInodeID = id;
    MemoryCopy(currentPath, canonical, StringLength(canonical) + 1);
    return TRUE;
}

void ListFiles(String option, String path) {
    Boolean all = FALSE;
    if (option && option[0]) {
        if (!StringEqual(option, "-a")) {
            PrintWithColor(LIGHT_RED, "ls: invalid option '%s'\n", option);
            return;
        }
        all = TRUE;
    }
    if (!path || !path[0]) path = ".";
    InodeID id;
    char canonical[FS_PATH_MAX];
    if (!resolvePath(path, &id, canonical, 0)) {
        printPathError("ls", path);
        return;
    }
    DiskInode* inode = readDiskInode(id);
    if (inode->Type != FT_DIRECTORY) {
        String name = canonical;
        for (u32 i = 0; canonical[i]; i++) if (canonical[i] == '/') name = canonical + i + 1;
        Printf("%s\n", name);
        Free(inode);
        return;
    }
    if ((inode->Permissions & (FS_OWNER_READ | FS_OWNER_EXECUTE)) != (FS_OWNER_READ | FS_OWNER_EXECUTE)) {
        Free(inode);
        lastPathError = PATH_PERMISSION;
        printPathError("ls", path);
        return;
    }
    Boolean printed = FALSE;
    if (all) {
        PrintWithColor(LIGHT_BLUE, "./\t../");
        printed = TRUE;
    }
    u8 buffer[512];
    u32 entries = inode->Size / sizeof(DirectoryEntry);
    for (u32 i = 0; i < entries; i++) {
        u32 offset = (i * sizeof(DirectoryEntry)) % 512;
        if (!offset) DiskCacheRead(inode->DirectBlock[i / (512 / sizeof(DirectoryEntry))], buffer);
        DirectoryEntry* entry = (DirectoryEntry*)(buffer + offset);
        if (entry->InodeID == INVALID_INODE || !entry->Name[0]) continue;
        entry->Name[FS_NAME_MAX - 1] = '\0';
        if (!all && entry->Name[0] == '.') continue;
        if (printed) Printf("\t");
        if (isDirectory(entry->InodeID)) PrintWithColor(LIGHT_BLUE, "%s/", entry->Name);
        else Printf("%s", entry->Name);
        printed = TRUE;
    }
    Printf("\n");
    Free(inode);
}

Boolean MakeDirectory(String path, String option) {
    u32 createMode = 1;
    if (option && option[0]) {
        if (!StringEqual(option, "-p")) {
            PrintWithColor(LIGHT_RED, "mkdir: invalid option '%s'\n", option);
            return FALSE;
        }
        createMode = 2;
    }
    if (!path || !path[0]) {
        lastPathError = PATH_INVALID;
        printPathError("mkdir", path);
        return FALSE;
    }
    InodeID id;
    char canonical[FS_PATH_MAX];
    if (!resolvePath(path, &id, canonical, createMode)) {
        printPathError("mkdir", path);
        return FALSE;
    }
    return TRUE;
}

Boolean SetFilePermissions(String path, u16 mode) {
    InodeID id;
    char canonical[FS_PATH_MAX];
    if (!resolvePath(path, &id, canonical, 0)) return FALSE;
    DiskInode* inode = readDiskInode(id);
    inode->Permissions = mode & 0777;
    writeDiskInode(id, inode);
    Free(inode);
    return TRUE;
}

InodeID CreateFile(InodeID id, String name, FileType type) {
    if (!validName(name) || (type != FT_FILE && type != FT_DIRECTORY)) {
        PrintWithColor(LIGHT_RED, "create: invalid or too long file name\n");
        return INVALID_INODE;
    }
    if (id == INVALID_INODE) id = currentDirectoryInodeID;
    DiskInode* dirInode = readDiskInode(id);
    if (dirInode->Type != FT_DIRECTORY) {
        PrintWithColor(LIGHT_RED, "create: %s: Not a directory\n", name);
        Free(dirInode);
        return INVALID_INODE;
    }
    if ((dirInode->Permissions & (FS_OWNER_WRITE | FS_OWNER_EXECUTE)) != (FS_OWNER_WRITE | FS_OWNER_EXECUTE)) {
        PrintWithColor(LIGHT_RED, "create: %s: Permission denied\n", name);
        Free(dirInode);
        return INVALID_INODE;
    }
    if (findInodeIDByNameFromDirectory(id, name) != INVALID_INODE) {
        PrintWithColor(LIGHT_RED, "create: %s: File exists\n", name);
        Free(dirInode);
        return INVALID_INODE;
    }
    u32 size = dirInode->Size;
    if (size + sizeof(DirectoryEntry) > INODE_DIRECT_BLOCKS * 512) {
        PrintWithColor(LIGHT_RED, "create: %s: Directory is full\n", name);
        Free(dirInode);
        return INVALID_INODE;
    }
    InodeID fileID = allocateOneInodeID();
    if (fileID == INVALID_INODE) {
        PrintWithColor(LIGHT_RED, "create: %s: No space left on device\n", name);
        Free(dirInode);
        return INVALID_INODE;
    }
    u8 buffer[512];
    if (size % 512 == 0) {
        dirInode->DirectBlock[size / 512] = allocateOneDataBlockID();
        if (!dirInode->DirectBlock[size / 512]) {
            freeOneInodeID(fileID);
            Free(dirInode);
            PrintWithColor(LIGHT_RED, "create: %s: No space left on device\n", name);
            return INVALID_INODE;
        }
        MemorySet(buffer, 0, sizeof(buffer));
    } else DiskCacheRead(dirInode->DirectBlock[size / 512], buffer);
    DiskInode* fileInode = newEmptyDiskInode();
    fileInode->Type = type;
    fileInode->RefCount = 1;
    fileInode->Permissions = type == FT_DIRECTORY ? 0755 : 0644;
    writeDiskInode(fileID, fileInode);
    DirectoryEntry* entry = newDirectoryEntry(name, fileID);
    MemoryCopy(buffer + size % 512, entry, sizeof(DirectoryEntry));
    DiskCacheWrite(dirInode->DirectBlock[size / 512], buffer);
    dirInode->Size += sizeof(DirectoryEntry);
    writeDiskInode(id, dirInode);
    Free(fileInode);
    Free(entry);
    Free(dirInode);
    return fileID;
}

Boolean RemoveFile(String option, String path) {
    if (option && option[0] && !StringEqual(option, "-r")) {
        Printf("rm: invalid option '%s'\n", option);
        return FALSE;
    }
    return RemovePath(path, option && StringEqual(option, "-r") ? FILE_RECURSIVE : 0);
}

void InitializeFileSystem() {
    easyFileSystem.Device = GetDeviceByID(1);
    easyFileSystem.rlock = NewReentrantLock();
    u32 totalBlocks = ((IdeDisk*)easyFileSystem.Device->DevicePtr)->TotalSector;
 
    // 1. 创建Inode 的位图，我们取5个块来存储inode位图，这样可以存储5*4096=20K个inode（文件）
    BitMap* inodeBitMap = newBitMap(1, 5);
    u32 inodeCount = 5 * 512 * 8;
    u32 inodeAreaBlocks = inodeCount / (512 / 128);

    easyFileSystem.InodeBitMap = inodeBitMap;
    easyFileSystem.InodeAreaStartBlockID = 5 + 1;

    /* 2. 创建数据块的位图，位图块数为a，数据块数为b，那么一定满足方程：
    *  a + b = remainBlocks 
    *  a * 512 * 8 = b
    * 所以我们推出 => a = remainBlocks / (512 * 8 + 1)
    */
    u32 remainBlocks = totalBlocks - 5 - 1 - inodeAreaBlocks; // 剩余的块数
    u32 dataBitMapBlocks = remainBlocks / (512 * 8 + 1);
    BitMap* dataBitMap = newBitMap(5 + 1 + inodeAreaBlocks, dataBitMapBlocks);
    u32 dataAreaBlocks = remainBlocks - dataBitMapBlocks;

    easyFileSystem.DataBitMap = dataBitMap;
    easyFileSystem.DataAreaStartBlockID = 5 + 1 + inodeAreaBlocks + dataBitMapBlocks;

    // Mount existing volumes and migrate legacy owner permissions without changing block offsets.
    u8 sector[512];
    DiskCacheRead(0, sector);
    SuperBlock* existing = (SuperBlock*)sector;
    Boolean mounted = existing->Magic == FS_MAGIC || existing->Magic == FS_LEGACY_MAGIC;
    if (mounted && existing->Magic == FS_LEGACY_MAGIC) {
        for (u32 block = 0; block < inodeBitMap->Blocks; block++) {
            u8 bitmap[512];
            DiskCacheRead(inodeBitMap->StartBlockID + block, bitmap);
            for (u32 byte = 0; byte < 512; byte++) {
                for (u32 bit = 0; bit < 8; bit++) {
                    if (!(bitmap[byte] & (1 << bit))) continue;
                    InodeID inodeID = block * 4096 + byte * 8 + bit;
                    DiskInode* inode = readDiskInode(inodeID);
                    inode->Permissions = inode->Type == FT_DIRECTORY ? 0755 : 0644;
                    writeDiskInode(inodeID, inode);
                    Free(inode);
                }
            }
        }
    }
    MemorySet(sector, 0, sizeof(sector));
    SuperBlock* superBlock = (SuperBlock*)sector;
    superBlock->DataAreaBlocks = dataAreaBlocks;
    superBlock->DataBitMapBlocks = dataBitMapBlocks;
    superBlock->InodeAreaBlocks = inodeAreaBlocks;
    superBlock->InodeBitMapBlocks = 5;
    superBlock->TotalBlocks = totalBlocks;
    superBlock->Magic = FS_MAGIC;
    DiskCacheWrite(0, sector);
    if (!mounted) {
        // Unformatted media may contain arbitrary bytes; initialize allocation metadata explicitly.
        MemorySet(sector, 0, sizeof(sector));
        for (u32 i = 0; i < inodeBitMap->Blocks; i++) DiskCacheWrite(inodeBitMap->StartBlockID + i, sector);
        for (u32 i = 0; i < dataBitMap->Blocks; i++) DiskCacheWrite(dataBitMap->StartBlockID + i, sector);
        InodeID rootID = allocateOneInodeID();
        DiskInode* rootInode = newEmptyDiskInode();
        rootInode->Type = FT_DIRECTORY;
        rootInode->RefCount = 1;
        rootInode->Permissions = 0755;
        writeDiskInode(rootID, rootInode);
        Free(rootInode);
    }
    currentDirectoryInodeID = 0;
    currentPath = (String)Malloc(FS_PATH_MAX);
    MemoryCopy(currentPath, "/", 2);
    // Home is available for cd and ~; the initial working directory remains /.
    MakeDirectory(FS_HOME_PATH, "-p");

}


// static methods implementation

static BitMap* newBitMap(u32 startBlockID, u32 blocks) {
    BitMap* bitmap = (BitMap*)Malloc(sizeof(BitMap));
    bitmap->StartBlockID = startBlockID;
    bitmap->Blocks = blocks;
    return bitmap;
}

static DiskInode* newEmptyDiskInode() {
    DiskInode* diskInode = (DiskInode*)Malloc(sizeof(DiskInode));
    diskInode->Type = FT_UNKNOWN;
    diskInode->Size = 0;
    diskInode->RefCount = 0;
    diskInode->Permissions = 0;
    for (u32 i = 0; i < INODE_DIRECT_BLOCKS; i++) {
        diskInode->DirectBlock[i] = 0;
    }
    diskInode->Indirect1 = 0;
    diskInode->Indirect2 = 0;

    return diskInode;
}

static void writeDiskInode(InodeID id, DiskInode* diskInode) {
    u32 blockID = easyFileSystem.InodeAreaStartBlockID + id / (512 / 128);
    u32 offset = (id % (512 / 128)) * 128;

    u8 buffer[512];
    DiskCacheRead(blockID, buffer);
    MemoryCopy(buffer + offset, diskInode, sizeof(DiskInode));
    DiskCacheWrite(blockID, buffer);
}

static DirectoryEntry* newDirectoryEntry(char* name, InodeID id) {
    DirectoryEntry* entry = (DirectoryEntry*)Malloc(sizeof(DirectoryEntry));
    entry->InodeID = id;
    MemorySet(entry->Name, 0, sizeof(entry->Name));
    MemoryCopy(entry->Name, name, StringLength(name) + 1);
    return entry;
}

static DiskInode* readDiskInode(InodeID id) {
    u32 blockID = easyFileSystem.InodeAreaStartBlockID + id / (512 / 128);
    u32 offset = (id % (512 / 128)) * 128;

    u8 buffer[512];
    DiskCacheRead(blockID, buffer);
    DiskInode* diskInode = (DiskInode*)Malloc(sizeof(DiskInode));
    MemoryCopy(diskInode, buffer + offset, sizeof(DiskInode));
    return diskInode;
}

static InodeID allocateOneInodeID() {
    easyFileSystem.rlock->Lock(easyFileSystem.rlock);

    for (u32 i = 0; i < easyFileSystem.InodeBitMap->Blocks; i++) {
        u32 blockID = easyFileSystem.InodeBitMap->StartBlockID + i;
        u8 buffer[512];
        DiskCacheRead(blockID, buffer);
        for (u32 j = 0; j < 512; j++) {
            u8 byte = buffer[j];
            for (u32 k = 0; k < 8; k++) {
                if ((byte & (1 << k)) == 0) {
                    byte |= (1 << k);
                    buffer[j] = byte;
                    DiskCacheWrite(blockID, buffer);
                    easyFileSystem.rlock->Unlock(easyFileSystem.rlock);
                    return i * 512 * 8 + j * 8 + k;
                }
            }
        }
    }

    easyFileSystem.rlock->Unlock(easyFileSystem.rlock);
    return INVALID_INODE;
}

static void freeOneInodeID(InodeID id) {
    easyFileSystem.rlock->Lock(easyFileSystem.rlock);

    u32 blockID = easyFileSystem.InodeBitMap->StartBlockID + id / (512 * 8);
    u32 offset = (id % (512 * 8)) / 8;
    u8 buffer[512];
    DiskCacheRead(blockID, buffer);
    u8 byte = buffer[offset];
    byte &= ~(1 << (id % 8));
    buffer[offset] = byte;
    DiskCacheWrite(blockID, buffer);

    easyFileSystem.rlock->Unlock(easyFileSystem.rlock);
}

static u32 allocateOneDataBlockID() {
    easyFileSystem.rlock->Lock(easyFileSystem.rlock);

    for (u32 i = 0; i < easyFileSystem.DataBitMap->Blocks; i++) {
        u32 blockID = easyFileSystem.DataBitMap->StartBlockID + i;
        u8 buffer[512];
        DiskCacheRead(blockID, buffer);
        for (u32 j = 0; j < 512; j++) {
            u8 byte = buffer[j];
            for (u32 k = 0; k < 8; k++) {
                if ((byte & (1 << k)) == 0) {
                    byte |= (1 << k);
                    buffer[j] = byte;
                    DiskCacheWrite(blockID, buffer);
                    easyFileSystem.rlock->Unlock(easyFileSystem.rlock);
                    return i * 512 * 8 + j * 8 + k + easyFileSystem.DataAreaStartBlockID;
                }
            }
        }
    }

    easyFileSystem.rlock->Unlock(easyFileSystem.rlock);
    return 0;
}

static void freeOneDataBlockID(u32 blockID) {
    easyFileSystem.rlock->Lock(easyFileSystem.rlock);

    // 清空内容
    u8 buffer[512];
    MemoryFree(buffer, 512);
    DiskCacheWrite(blockID, buffer);

    // 清空位图的指定位
    u32 id = blockID - easyFileSystem.DataAreaStartBlockID;
    u32 blockID2 = easyFileSystem.DataBitMap->StartBlockID + id / (512 * 8);
    u32 offset = (id % (512 * 8)) / 8;
    DiskCacheRead(blockID2, buffer);
    u8 byte = buffer[offset];
    byte &= ~(1 << (id % 8));
    buffer[offset] = byte;
    DiskCacheWrite(blockID2, buffer);

    easyFileSystem.rlock->Unlock(easyFileSystem.rlock);
}

static InodeID findInodeIDByNameFromDirectory(InodeID id, String name) {
    if (id == INVALID_INODE || !validName(name)) return INVALID_INODE;
    DiskInode* directory = readDiskInode(id);
    if (directory->Type != FT_DIRECTORY) { Free(directory); return INVALID_INODE; }
    u8 buffer[512];
    u32 entries = directory->Size / sizeof(DirectoryEntry);
    for (u32 i = 0; i < entries; i++) {
        u32 offset = (i * sizeof(DirectoryEntry)) % 512;
        if (!offset) DiskCacheRead(directory->DirectBlock[i / (512 / sizeof(DirectoryEntry))], buffer);
        DirectoryEntry* entry = (DirectoryEntry*)(buffer + offset);
        entry->Name[FS_NAME_MAX - 1] = '\0';
        if (entry->InodeID != INVALID_INODE && StringEqual(entry->Name, name)) {
            InodeID result = entry->InodeID;
            Free(directory);
            return result;
        }
    }
    Free(directory);
    return INVALID_INODE;
}

static Boolean isDirectory(InodeID id) {
    if (id == INVALID_INODE) return FALSE;
    Boolean result = FALSE;
    DiskInode* diskInode = readDiskInode(id);
    result = diskInode->Type == FT_DIRECTORY;

    Free(diskInode);
    return result;
}

static Boolean isFile(InodeID id) {
    if (id == INVALID_INODE) return FALSE;
    Boolean result = FALSE;
    DiskInode* diskInode = readDiskInode(id);
    result = diskInode->Type == FT_FILE;

    Free(diskInode);
    return result;
}

static u32 boundedLength(String text, u32 limit) {
    if (!text) return limit;
    u32 size = 0;
    while (size < limit && text[size]) size++;
    return size;
}

static Boolean validName(String name) {
    u32 length = boundedLength(name, FS_NAME_MAX);
    if (!length || length == FS_NAME_MAX || StringEqual(name, ".") || StringEqual(name, "..")) return FALSE;
    for (u32 i = 0; i < length; i++) if (name[i] == '/' || (u8)name[i] < 32 || name[i] == 127) return FALSE;
    return TRUE;
}

static Boolean hasPermission(InodeID id, u16 permission) {
    if (id == INVALID_INODE) return FALSE;
    DiskInode* inode = readDiskInode(id);
    Boolean allowed = (inode->Permissions & permission) == permission;
    Free(inode);
    return allowed;
}

static void printPathError(String command, String path) {
    String reason = "Invalid path";
    if (lastPathError == PATH_MISSING) reason = "No such file or directory";
    else if (lastPathError == PATH_NOT_DIRECTORY) reason = "Not a directory";
    else if (lastPathError == PATH_TOO_LONG) reason = "Path too long";
    else if (lastPathError == PATH_NAME_TOO_LONG) reason = "File name too long";
    else if (lastPathError == PATH_EXISTS) reason = "File exists";
    else if (lastPathError == PATH_PERMISSION) reason = "Permission denied";
    // The rejected input need not be NUL terminated inside FS_PATH_MAX.
    if (lastPathError == PATH_TOO_LONG) PrintWithColor(LIGHT_RED, "%s: %s\n", command, reason);
    else PrintWithColor(LIGHT_RED, "%s: %s: %s\n", command, path ? path : "", reason);
}

/* Resolve components in order: a file followed by /.. still fails as a non-directory. */
static Boolean resolvePath(String path, InodeID* result, String canonical, u32 createMode) {
    lastPathError = PATH_OK;
    u32 length = boundedLength(path, FS_PATH_MAX);
    if (!length) {
        lastPathError = PATH_MISSING;
        return FALSE;
    }
    if (length == FS_PATH_MAX) {
        lastPathError = PATH_TOO_LONG;
        return FALSE;
    }
    char absolute[FS_PATH_MAX * 2];
    u32 used = 0;
    String tail = path;
    if (path[0] == '~') {
        if (path[1] && path[1] != '/') {
            lastPathError = PATH_INVALID;
            return FALSE;
        }
        used = StringLength(FS_HOME_PATH);
        MemoryCopy(absolute, FS_HOME_PATH, used);
        tail = path + 1;
    } else if (path[0] != '/') {
        used = StringLength(currentPath);
        MemoryCopy(absolute, currentPath, used);
        if (used > 1) absolute[used++] = '/';
    }
    MemoryCopy(absolute + used, tail, StringLength(tail) + 1);
    u32 absoluteLength = used + StringLength(tail);
    canonical[0] = '/';
    canonical[1] = '\0';
    u32 canonicalLength = 1;
    InodeID parents[FS_PATH_MAX / 2];
    u32 depth = 0;
    InodeID id = 0;
    Boolean createdFinal = FALSE;
    for (u32 position = 1; absolute[position];) {
        while (absolute[position] == '/') position++;
        if (!absolute[position]) break;
        char name[FS_NAME_MAX];
        u32 count = 0;
        while (absolute[position] && absolute[position] != '/') {
            if (count + 1 >= FS_NAME_MAX) {
                lastPathError = PATH_NAME_TOO_LONG;
                return FALSE;
            }
            name[count++] = absolute[position++];
        }
        name[count] = '\0';
        u32 next = position;
        while (absolute[next] == '/') next++;
        Boolean final = absolute[next] == '\0';
        if (!isDirectory(id)) {
            lastPathError = PATH_NOT_DIRECTORY;
            return FALSE;
        }
        if (!hasPermission(id, FS_OWNER_EXECUTE)) {
            lastPathError = PATH_PERMISSION;
            return FALSE;
        }
        if (StringEqual(name, ".")) continue;
        if (StringEqual(name, "..")) {
            if (depth) {
                id = parents[--depth];
                pathBackStep(canonical);
                canonicalLength = StringLength(canonical);
            }
            continue;
        }
        if (!validName(name)) {
            lastPathError = PATH_INVALID;
            return FALSE;
        }
        u32 addition = count + (canonicalLength > 1 ? 1 : 0);
        if (canonicalLength + addition >= FS_PATH_MAX) {
            lastPathError = PATH_TOO_LONG;
            return FALSE;
        }
        InodeID nextID = findInodeIDByNameFromDirectory(id, name);
        if (nextID == INVALID_INODE) {
            if (!createMode || (!final && createMode != 2)) {
                lastPathError = PATH_MISSING;
                return FALSE;
            }
            if (!hasPermission(id, FS_OWNER_WRITE | FS_OWNER_EXECUTE)) {
                lastPathError = PATH_PERMISSION;
                return FALSE;
            }
            nextID = CreateFile(id, name, FT_DIRECTORY);
            if (nextID == INVALID_INODE) {
                lastPathError = PATH_INVALID;
                return FALSE;
            }
            createdFinal = final;
        }
        if (createMode && !isDirectory(nextID)) {
            lastPathError = final && createMode == 1 ? PATH_EXISTS : PATH_NOT_DIRECTORY;
            return FALSE;
        }
        parents[depth++] = id;
        id = nextID;
        if (canonicalLength > 1) canonical[canonicalLength++] = '/';
        MemoryCopy(canonical + canonicalLength, name, count + 1);
        canonicalLength += count;
    }
    if ((createMode || absolute[absoluteLength - 1] == '/') && !isDirectory(id)) {
        lastPathError = PATH_NOT_DIRECTORY;
        return FALSE;
    }
    if (createMode == 1 && !createdFinal) {
        lastPathError = PATH_EXISTS;
        return FALSE;
    }
    *result = id;
    return TRUE;
}

static InodeID pathToInodeID(String path) {
    InodeID id;
    char canonical[FS_PATH_MAX];
    return resolvePath(path, &id, canonical, 0) ? id : INVALID_INODE;
}

static void pathBackStep(String path) {
    u32 length = StringLength(path);
    while (length > 1 && path[length - 1] != '/') length--;
    if (length > 1) length--;
    path[length] = '\0';
}

/* File operations reuse the original inode layout, path resolver and EOF model. */
#define FILE_WALK_DEPTH (FS_PATH_MAX / 2)
static const char* fileCommand;
typedef struct FileNode {
    InodeID id, parent;
    char name[FS_NAME_MAX];
    char path[FS_PATH_MAX];
} FileNode;
typedef struct FileWalk {
    InodeID id, parent, destination;
    u32 next, sourceLength, targetLength;
    Boolean prompted, created;
    char name[FS_NAME_MAX];
} FileWalk;

static Boolean fileError(const char* path, const char* reason) {
    PrintWithColor(LIGHT_RED, "%s: %s: %s\n", fileCommand, path ? path : "", reason);
    return FALSE;
}
static void fileText(char* output, const char* text) {
    MemoryCopy(output, text, StringLength(text) + 1);
}
static Boolean fileAncestor(const char* ancestor, const char* path) {
    u32 size = StringLength(ancestor);
    if (StringEqual(ancestor, "/")) return TRUE;
    for (u32 i = 0; i < size; i++) if (path[i] != ancestor[i]) return FALSE;
    return !path[size] || path[size] == '/';
}
static Boolean fileAppend(char* path, const char* name) {
    u32 size = StringLength(path), length = StringLength(name);
    if (size + (size > 1) + length >= FS_PATH_MAX) return fileError(path, "Path too long");
    if (size > 1) path[size++] = '/';
    MemoryCopy(path + size, name, length + 1);
    return TRUE;
}
static Boolean fileSpecialLast(const char* path) {
    u32 length = boundedLength((String)path, FS_PATH_MAX);
    if (length == FS_PATH_MAX) return FALSE;
    while (length && path[length - 1] == '/') length--;
    u32 start = length;
    while (start && path[start - 1] != '/') start--;
    return (length - start == 1 && path[start] == '.') ||
           (length - start == 2 && path[start] == '.' && path[start + 1] == '.');
}
static Boolean fileAccess(InodeID id, u16 required, Boolean directory, const char* path) {
    if (directory && !isDirectory(id)) return fileError(path, "Not a directory");
    if (!hasPermission(id, required)) return fileError(path, "Permission denied");
    return TRUE;
}
static Boolean fileResolve(String path, FileNode* node, Boolean missing) {
    char parentPath[FS_PATH_MAX];
    u32 length = boundedLength(path, FS_PATH_MAX);
    if (!length) return fileError(path, "Invalid path");
    if (length == FS_PATH_MAX) return fileError("", "Path too long");
    if (resolvePath(path, &node->id, node->path, 0)) {
        if (!node->id) {
            node->parent = 0;
            node->name[0] = '\0';
            return TRUE;
        }
        u32 start = StringLength(node->path);
        while (start && node->path[start - 1] != '/') start--;
        fileText(node->name, node->path + start);
        fileText(parentPath, node->path);
        pathBackStep(parentPath);
        node->parent = pathToInodeID(parentPath);
        return node->parent != INVALID_INODE;
    }
    if (!missing || lastPathError != PATH_MISSING || path[length - 1] == '/') {
        printPathError((String)fileCommand, path);
        return FALSE;
    }
    i32 slash = -1;
    for (u32 i = 0; i < length; i++) if (path[i] == '/') slash = i;
    String name = path + slash + 1;
    if (!validName(name)) return fileError(path, "Invalid or too long file name");
    if (slash < 0) fileText(parentPath, ".");
    else if (!slash) fileText(parentPath, "/");
    else { MemoryCopy(parentPath, path, slash); parentPath[slash] = '\0'; }
    if (!resolvePath(parentPath, &node->parent, node->path, 0)) {
        printPathError((String)fileCommand, path);
        return FALSE;
    }
    if (!fileAccess(node->parent, FS_OWNER_EXECUTE, TRUE, parentPath)) return FALSE;
    if (!fileAppend(node->path, name)) return FALSE;
    fileText(node->name, name);
    node->id = INVALID_INODE;
    return TRUE;
}
static Boolean fileReadEntry(const DiskInode* inode, u32 index, DirectoryEntry* entry) {
    if (index >= inode->Size / sizeof(DirectoryEntry) || index >= INODE_DIRECT_BLOCKS * 16) return FALSE;
    u8 sector[512];
    u32 offset = index * sizeof(DirectoryEntry);
    DiskCacheRead(inode->DirectBlock[offset / 512], sector);
    MemoryCopy(entry, sector + offset % 512, sizeof(*entry));
    entry->Name[FS_NAME_MAX - 1] = '\0';
    return TRUE;
}
static void fileWriteEntry(const DiskInode* inode, u32 index, const DirectoryEntry* entry) {
    u8 sector[512];
    u32 offset = index * sizeof(DirectoryEntry), block = inode->DirectBlock[offset / 512];
    DiskCacheRead(block, sector);
    MemoryCopy(sector + offset % 512, entry, sizeof(*entry));
    DiskCacheWrite(block, sector);
}
static Boolean fileAddEntry(InodeID parent, String name, InodeID id) {
    DiskInode* inode = readDiskInode(parent);
    if (inode->Size + sizeof(DirectoryEntry) > INODE_DIRECT_BLOCKS * 512) {
        Free(inode);
        return fileError(name, "No space left on device");
    }
    if (!(inode->Size % 512)) {
        u32 block = allocateOneDataBlockID();
        if (!block) { Free(inode); return fileError(name, "No space left on device"); }
        inode->DirectBlock[inode->Size / 512] = block;
        u8 sector[512];
        MemorySet(sector, 0, sizeof(sector));
        DiskCacheWrite(block, sector);
    }
    DirectoryEntry entry;
    MemorySet(&entry, 0, sizeof(entry));
    entry.InodeID = id;
    fileText(entry.Name, name);
    fileWriteEntry(inode, inode->Size / sizeof(entry), &entry);
    inode->Size += sizeof(entry);
    writeDiskInode(parent, inode);
    Free(inode);
    return TRUE;
}
/* Detach a packed directory entry without freeing the referenced inode. */
static Boolean fileDetach(InodeID parent, String name) {
    DiskInode* inode = readDiskInode(parent);
    DirectoryEntry entry;
    u32 count = inode->Size / sizeof(entry), index = count;
    for (u32 i = 0; i < count; i++) {
        fileReadEntry(inode, i, &entry);
        if (StringEqual(entry.Name, name)) { index = i; break; }
    }
    if (index == count) { Free(inode); return FALSE; }
    fileReadEntry(inode, count - 1, &entry);
    if (index != count - 1) fileWriteEntry(inode, index, &entry);
    MemorySet(&entry, 0, sizeof(entry));
    fileWriteEntry(inode, count - 1, &entry);
    inode->Size -= sizeof(entry);
    if (!(inode->Size % 512)) {
        freeOneDataBlockID(inode->DirectBlock[inode->Size / 512]);
        inode->DirectBlock[inode->Size / 512] = 0;
    }
    writeDiskInode(parent, inode);
    Free(inode);
    return TRUE;
}
static void fileDestroy(InodeID id) {
    DiskInode* inode = readDiskInode(id);
    for (u32 i = 0; i < INODE_DIRECT_BLOCKS; i++) if (inode->DirectBlock[i]) freeOneDataBlockID(inode->DirectBlock[i]);
    MemorySet(inode, 0, sizeof(*inode));
    writeDiskInode(id, inode);
    freeOneInodeID(id);
    Free(inode);
}
static Boolean fileConfirm(const char* action, const char* path) {
    Printf("%s '%s'? [y/N] ", action, path);
    char answer[32];
    u32 length = 0, excess = 0;
    for (;;) {
        char ch = ReadChar();
        if (ch == 3) { Printf("^C\n"); return FALSE; }
        if (ch == '\n' || ch == '\r') { Printf("\n"); break; }
        if (ch == '\b') {
            if (excess) excess--;
            else if (length) { length--; Printf("\b"); }
        } else if ((ch >= ' ' && ch <= '~') || ch == '\t') {
            if (length + 1 < sizeof(answer) && !excess) {
                answer[length++] = ch;
                Printf("%c", ch);
            } else excess++;
        }
    }
    if (excess) return FALSE;
    u32 first = 0;
    while (first < length && (answer[first] == ' ' || answer[first] == '\t')) first++;
    return first < length && (answer[first] == 'y' || answer[first] == 'Y');
}
static Boolean fileRemoveTree(const FileNode* root, u32 flags) {
    DiskInode* inode = readDiskInode(root->id);
    if (inode->Type == FT_DIRECTORY && !(flags & (FILE_RECURSIVE | FILE_DIRECTORY))) {
        Free(inode);
        return fileError(root->path, "Is a directory");
    }
    if (inode->Type == FT_DIRECTORY && inode->Size && !(flags & FILE_RECURSIVE)) {
        Free(inode);
        return fileError(root->path, "Directory not empty");
    }
    Free(inode);
    FileWalk* frames = (FileWalk*)Malloc(FILE_WALK_DEPTH * sizeof(FileWalk));
    char* path = (char*)Malloc(FS_PATH_MAX);
    MemorySet(frames, 0, FILE_WALK_DEPTH * sizeof(FileWalk));
    fileText(path, root->path);
    frames[0].id = root->id;
    frames[0].parent = root->parent;
    frames[0].sourceLength = StringLength(path);
    fileText(frames[0].name, root->name);
    u32 depth = 0;
    Boolean success = TRUE;
    for (;;) {
        FileWalk* frame = frames + depth;
        path[frame->sourceLength] = '\0';
        if (!fileAccess(frame->parent, FS_OWNER_WRITE | FS_OWNER_EXECUTE, TRUE, path)) { success = FALSE; break; }
        inode = readDiskInode(frame->id);
        Boolean directory = inode->Type == FT_DIRECTORY;
        if (!frame->prompted && (flags & FILE_INTERACTIVE)) {
            frame->prompted = TRUE;
            if (!fileConfirm(directory ? "remove directory" : "remove", path)) {
                Free(inode);
                if (!depth) break;
                depth--;
                frames[depth].next++;
                continue;
            }
        }
        if (directory && frame->next < inode->Size / sizeof(DirectoryEntry)) {
            if (!fileAccess(frame->id, FS_OWNER_READ | FS_OWNER_WRITE | FS_OWNER_EXECUTE, TRUE, path)) {
                Free(inode);
                success = FALSE;
                break;
            }
            DirectoryEntry entry;
            fileReadEntry(inode, frame->next, &entry);
            Free(inode);
            if (!entry.Name[0] || !entry.InodeID || entry.InodeID == INVALID_INODE) {
                frame->next++;
                continue;
            }
            if (depth + 1 >= FILE_WALK_DEPTH || !fileAppend(path, entry.Name)) { success = FALSE; break; }
            FileWalk* child = frames + ++depth;
            MemorySet(child, 0, sizeof(*child));
            child->id = entry.InodeID;
            child->parent = frame->id;
            child->sourceLength = StringLength(path);
            fileText(child->name, entry.Name);
            continue;
        }
        Boolean retained = directory && inode->Size;
        Free(inode);
        if (retained) {
            if (!depth) break;
            depth--;
            frames[depth].next++;
            continue;
        }
        if (!fileDetach(frame->parent, frame->name)) { success = fileError(path, "No such file or directory"); break; }
        fileDestroy(frame->id);
        if (flags & FILE_VERBOSE) Printf("removed '%s'\n", path);
        if (!depth) break;
        depth--;
        /* Detaching packs entries, so this parent's next index does not advance. */
    }
    Free(path);
    Free(frames);
    return success;
}

Boolean RemovePath(String path, u32 flags) {
    easyFileSystem.rlock->Lock(easyFileSystem.rlock);
    fileCommand = "rm";
    FileNode* node = (FileNode*)Malloc(sizeof(FileNode));
    Boolean success;
    if ((flags & FILE_FORCE) && boundedLength(path, FS_PATH_MAX) < FS_PATH_MAX && path && path[0]) {
        InodeID id;
        char canonical[FS_PATH_MAX];
        if (!resolvePath(path, &id, canonical, 0) && lastPathError == PATH_MISSING) {
            Free(node);
            easyFileSystem.rlock->Unlock(easyFileSystem.rlock);
            return TRUE;
        }
    }
    success = fileResolve(path, node, FALSE);
    if (success && (!node->id || fileSpecialLast(path) || fileAncestor(node->path, currentPath)))
        success = fileError(path, "Cannot remove root, dot, or a working-directory ancestor");
    if (success) success = fileRemoveTree(node, flags);
    Free(node);
    easyFileSystem.rlock->Unlock(easyFileSystem.rlock);
    return success;
}

static Boolean fileCopyRegular(InodeID source, InodeID parent, String name, const char* path, u32 flags,
                               InodeID* destination, Boolean* skipped) {
    *skipped = FALSE;
    if (!fileAccess(source, FS_OWNER_READ, FALSE, path)) return FALSE;
    DiskInode* input = readDiskInode(source);
    InodeID target = findInodeIDByNameFromDirectory(parent, name);
    DiskInode* old = target == INVALID_INODE ? NULL : readDiskInode(target);
    if (target == source) { Free(input); if (old) Free(old); return fileError(path, "Source and destination are the same file"); }
    if (old && old->Type == FT_DIRECTORY) { Free(input); Free(old); return fileError(path, "Is a directory"); }
    if (old && ((flags & FILE_NO_CLOBBER) || ((flags & FILE_INTERACTIVE) && !fileConfirm("overwrite", path)))) {
        *destination = target;
        *skipped = TRUE;
        Free(input);
        Free(old);
        return TRUE;
    }
    Boolean created = target == INVALID_INODE;
    if (created && !fileAccess(parent, FS_OWNER_WRITE | FS_OWNER_EXECUTE, TRUE, path)) {
        Free(input);
        return FALSE;
    }
    Boolean replace = old && !(old->Permissions & FS_OWNER_WRITE);
    if (replace && (!(flags & FILE_FORCE) || !hasPermission(parent, FS_OWNER_WRITE | FS_OWNER_EXECUTE))) {
        Free(input);
        Free(old);
        return fileError(path, "Permission denied");
    }
    if (input->Size > INODE_DIRECT_BLOCKS * 512) {
        Free(input);
        if (old) Free(old);
        return fileError(path, "File too large");
    }
    DiskInode* pending = newEmptyDiskInode();
    pending->Type = FT_FILE;
    pending->RefCount = old && !replace ? old->RefCount : 1;
    pending->Permissions = old && !replace ? old->Permissions : input->Permissions;
    pending->Size = input->Size;
    u32 blocks = (input->Size + 511) / 512;
    u32 oldBlocks = old && !replace ? (old->Size + 511) / 512 : 0;
    if (oldBlocks > INODE_DIRECT_BLOCKS) oldBlocks = INODE_DIRECT_BLOCKS;
    u32 reused = oldBlocks < blocks ? oldBlocks : blocks;
    for (u32 i = 0; i < reused; i++) pending->DirectBlock[i] = old->DirectBlock[i];
    u32 reserved = reused;
    /* Reserve only growth before touching reused blocks, preserving the old file on ENOSPC. */
    while (reserved < blocks) {
        u32 block = allocateOneDataBlockID();
        if (!block) break;
        pending->DirectBlock[reserved++] = block;
    }
    Boolean success = reserved == blocks;
    if (!success) fileError(path, "No space left on device");
    InodeID newID = target;
    if (success && created) {
        newID = CreateFile(parent, name, FT_FILE);
        success = newID != INVALID_INODE;
    } else if (success && replace) {
        newID = allocateOneInodeID();
        if (newID == INVALID_INODE) success = fileError(path, "No space left on device");
    }
    if (success) {
        u8 sector[512];
        for (u32 i = 0; i < blocks; i++) {
            DiskCacheRead(input->DirectBlock[i], sector);
            DiskCacheWrite(pending->DirectBlock[i], sector);
        }
        writeDiskInode(newID, pending);
        if (replace) {
            DiskInode* directory = readDiskInode(parent);
            DirectoryEntry entry;
            for (u32 index = 0; fileReadEntry(directory, index, &entry); index++) {
                if (!StringEqual(entry.Name, name)) continue;
                entry.InodeID = newID;
                fileWriteEntry(directory, index, &entry);
                break;
            }
            Free(directory);
            fileDestroy(target);
        } else if (old) {
            for (u32 i = blocks; i < INODE_DIRECT_BLOCKS; i++)
                if (old->DirectBlock[i]) freeOneDataBlockID(old->DirectBlock[i]);
        }
        *destination = newID;
        if (flags & FILE_VERBOSE) Printf("copied '%s'\n", path);
    } else {
        for (u32 i = reused; i < reserved; i++) freeOneDataBlockID(pending->DirectBlock[i]);
    }
    Free(pending);
    Free(input);
    if (old) Free(old);
    return success;
}
static Boolean fileCopyTree(const FileNode* source, const FileNode* target, u32 flags) {
    DiskInode* inode = readDiskInode(source->id);
    if (inode->Type == FT_FILE) {
        Free(inode);
        InodeID destination;
        Boolean skipped;
        return fileCopyRegular(source->id, target->parent, (String)target->name, target->path, flags, &destination, &skipped);
    }
    if (!(flags & FILE_RECURSIVE)) { Free(inode); return fileError(source->path, "Omitting directory (use -r)"); }
    if (!fileAccess(source->id, FS_OWNER_READ | FS_OWNER_EXECUTE, TRUE, source->path)) { Free(inode); return FALSE; }
    InodeID destination = target->id;
    Boolean created = destination == INVALID_INODE;
    if (created) {
        destination = CreateFile(target->parent, (String)target->name, FT_DIRECTORY);
        if (destination == INVALID_INODE) { Free(inode); return FALSE; }
    } else if (!fileAccess(destination, FS_OWNER_EXECUTE, TRUE, target->path)) { Free(inode); return FALSE; }
    Free(inode);
    FileWalk* frames = (FileWalk*)Malloc(FILE_WALK_DEPTH * sizeof(FileWalk));
    char* path = (char*)Malloc(FS_PATH_MAX);
    fileText(path, target->path);
    MemorySet(frames, 0, FILE_WALK_DEPTH * sizeof(FileWalk));
    frames[0].id = source->id;
    frames[0].destination = destination;
    frames[0].targetLength = StringLength(path);
    frames[0].created = created;
    u32 depth = 0;
    Boolean success = TRUE;
    for (;;) {
        FileWalk* frame = frames + depth;
        path[frame->targetLength] = '\0';
        inode = readDiskInode(frame->id);
        if (!fileAccess(frame->id, FS_OWNER_READ | FS_OWNER_EXECUTE, TRUE, path)) { Free(inode); success = FALSE; break; }
        if (frame->next >= inode->Size / sizeof(DirectoryEntry)) {
            if (frame->created) {
                DiskInode* output = readDiskInode(frame->destination);
                output->Permissions = inode->Permissions;
                writeDiskInode(frame->destination, output);
                Free(output);
            }
            Free(inode);
            if (!depth) break;
            depth--;
            continue;
        }
        DirectoryEntry entry;
        fileReadEntry(inode, frame->next++, &entry);
        Free(inode);
        if (!entry.Name[0] || !entry.InodeID || entry.InodeID == INVALID_INODE) continue;
        if (!fileAppend(path, entry.Name)) { success = FALSE; break; }
        inode = readDiskInode(entry.InodeID);
        Boolean directory = inode->Type == FT_DIRECTORY;
        Free(inode);
        InodeID childID = findInodeIDByNameFromDirectory(frame->destination, entry.Name);
        if (childID == entry.InodeID) { success = fileError(path, "Source and destination are the same file"); break; }
        if (!directory) {
            Boolean skipped;
            success = fileCopyRegular(entry.InodeID, frame->destination, entry.Name, path, flags, &childID, &skipped);
            if (!success) break;
            continue;
        }
        Boolean newChild = childID == INVALID_INODE;
        if (newChild) {
            childID = CreateFile(frame->destination, entry.Name, FT_DIRECTORY);
            if (childID == INVALID_INODE) { success = FALSE; break; }
            if (flags & FILE_VERBOSE) Printf("created directory '%s'\n", path);
        } else if (!fileAccess(childID, FS_OWNER_EXECUTE, TRUE, path)) { success = FALSE; break; }
        if (depth + 1 >= FILE_WALK_DEPTH) { success = fileError(path, "Path too long"); break; }
        FileWalk* child = frames + ++depth;
        MemorySet(child, 0, sizeof(*child));
        child->id = entry.InodeID;
        child->destination = childID;
        child->targetLength = StringLength(path);
        child->created = newChild;
    }
    Free(path);
    Free(frames);
    return success;
}

static Boolean fileMoveNode(const FileNode* source, const FileNode* target, u32 flags) {
    if (!fileAccess(source->parent, FS_OWNER_WRITE | FS_OWNER_EXECUTE, TRUE, source->path) ||
        !fileAccess(target->parent, FS_OWNER_WRITE | FS_OWNER_EXECUTE, TRUE, target->path)) return FALSE;
    DiskInode* input = readDiskInode(source->id);
    DiskInode* output = target->id == INVALID_INODE ? NULL : readDiskInode(target->id);
    Boolean success = TRUE;
    if (output) {
        if (input->Type == FT_DIRECTORY && output->Type != FT_DIRECTORY) success = fileError(target->path, "Not a directory");
        else if (input->Type != FT_DIRECTORY && output->Type == FT_DIRECTORY) success = fileError(target->path, "Is a directory");
        else if (output->Type == FT_DIRECTORY && output->Size) success = fileError(target->path, "Directory not empty");
        else if (fileAncestor(target->path, currentPath)) success = fileError(target->path, "Destination is a working-directory ancestor");
        if (success && ((flags & FILE_NO_CLOBBER) || ((flags & FILE_INTERACTIVE) && !fileConfirm("overwrite", target->path)))) {
            Free(input);
            Free(output);
            return TRUE;
        }
    }
    if (output) Free(output);
    Free(input);
    char* newPath = (char*)Malloc(FS_PATH_MAX);
    fileText(newPath, currentPath);
    if (success && fileAncestor(source->path, currentPath)) {
        const char* suffix = currentPath + StringLength(source->path);
        if (StringLength(target->path) + StringLength(suffix) >= FS_PATH_MAX) success = fileError(target->path, "Path too long");
        else {
            fileText(newPath, target->path);
            fileText(newPath + StringLength(newPath), suffix);
        }
    }
    if (success && target->id == INVALID_INODE && source->parent == target->parent) {
        DiskInode* parent = readDiskInode(source->parent);
        DirectoryEntry entry;
        for (u32 index = 0; fileReadEntry(parent, index, &entry); index++) {
            if (!StringEqual(entry.Name, source->name)) continue;
            MemorySet(entry.Name, 0, sizeof(entry.Name));
            fileText(entry.Name, target->name);
            fileWriteEntry(parent, index, &entry);
            break;
        }
        Free(parent);
    } else if (success) {
        if (target->id == INVALID_INODE) success = fileAddEntry(target->parent, (String)target->name, source->id);
        else {
            DiskInode* parent = readDiskInode(target->parent);
            DirectoryEntry entry;
            for (u32 index = 0; fileReadEntry(parent, index, &entry); index++) {
                if (!StringEqual(entry.Name, target->name)) continue;
                entry.InodeID = source->id;
                fileWriteEntry(parent, index, &entry);
                break;
            }
            Free(parent);
        }
        if (success) {
            success = fileDetach(source->parent, (String)source->name);
            if (success && target->id != INVALID_INODE) fileDestroy(target->id);
        }
    }
    if (success) {
        fileText(currentPath, newPath);
        if (flags & FILE_VERBOSE) Printf("renamed '%s' -> '%s'\n", source->path, target->path);
    }
    Free(newPath);
    return success;
}

static Boolean fileTransfer(String sourcePath, String targetPath, u32 flags, Boolean move) {
    FileNode* source = (FileNode*)Malloc(sizeof(FileNode));
    FileNode* target = (FileNode*)Malloc(sizeof(FileNode));
    Boolean success = fileResolve(sourcePath, source, FALSE) && fileResolve(targetPath, target, TRUE);
    if (!success) {
        Free(target);
        Free(source);
        return FALSE;
    }
    if (success && (!source->id || (move && fileSpecialLast(sourcePath))))
        success = fileError(sourcePath, "Cannot use root, dot, or dot-dot as the source");
    u32 sourceLength = boundedLength(sourcePath, FS_PATH_MAX);
    while (sourceLength && sourcePath[sourceLength - 1] == '/') sourceLength--;
    Boolean mergeContents = !move && sourceLength && sourcePath[sourceLength - 1] == '.' &&
                            (sourceLength == 1 || sourcePath[sourceLength - 2] == '/');
    Boolean targetDirectory = success && target->id != INVALID_INODE && isDirectory(target->id);
    if (success && (flags & FILE_TARGET_DIRECTORY) && !targetDirectory) success = fileError(targetPath, "Target must be an existing directory");
    if (success && targetDirectory && !(flags & FILE_NO_TARGET_DIR) && !mergeContents) {
        target->parent = target->id;
        success = fileAccess(target->parent, FS_OWNER_EXECUTE, TRUE, target->path) && fileAppend(target->path, source->name);
        if (success) {
            fileText(target->name, source->name);
            target->id = findInodeIDByNameFromDirectory(target->parent, target->name);
        }
    }
    if (success && source->id == target->id) success = fileError(target->path, "Source and destination are the same file");
    if (success && isDirectory(source->id) && fileAncestor(source->path, target->path))
        success = fileError(target->path, "Cannot place a directory inside itself");
    if (success) success = move ? fileMoveNode(source, target, flags) : fileCopyTree(source, target, flags);
    Free(target);
    Free(source);
    return success;
}
Boolean CopyPath(String source, String target, u32 flags) {
    easyFileSystem.rlock->Lock(easyFileSystem.rlock);
    fileCommand = "cp";
    Boolean success = fileTransfer(source, target, flags, FALSE);
    easyFileSystem.rlock->Unlock(easyFileSystem.rlock);
    return success;
}
Boolean MovePath(String source, String target, u32 flags) {
    easyFileSystem.rlock->Lock(easyFileSystem.rlock);
    fileCommand = "mv";
    Boolean success = fileTransfer(source, target, flags, TRUE);
    easyFileSystem.rlock->Unlock(easyFileSystem.rlock);
    return success;
}
