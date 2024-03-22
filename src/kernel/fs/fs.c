#include "mod.h"

/**
 * 我们第一版先不做文件系统的一级间接和二级间接。后面会进行补充 TODO
*/

// global variables
static EasyFS easyFileSystem;
static InodeID currentDirectoryInodeID; // 默认为根目录
static String currentPath = NULL; // 默认为根目录

// static methods declaration

static BitMap* newBitMap(u32 startBlockID, u32 blocks);
static DiskInode* newEmptyDiskInode();
static DirectoryEntry* newDirectoryEntry(char* name, InodeID id);
static InodeID allocateOneInodeID();
static void freeOneInodeID(InodeID id);
static u32 allocateOneDataBlockID();
static void freeOneDataBlockID(u32 blockID);
static void writeDiskInode(InodeID id, DiskInode* diskInode);
static DiskInode* readDiskInode(InodeID id);
static InodeID findInodeIDByNameFromDirectory(InodeID id, String name);
static Boolean removeInodeByNameFromDirectory(InodeID id, String name);
static Boolean isDirectory(InodeID id);
static Boolean isFile(InodeID id);

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

    // 3. 创建超级块
    SuperBlock* superBlock = (SuperBlock*)AllocateOnePage(KernelMode);
    superBlock->DataAreaBlocks = dataAreaBlocks;
    superBlock->DataBitMapBlocks = dataBitMapBlocks;
    superBlock->InodeAreaBlocks = inodeAreaBlocks;
    superBlock->InodeBitMapBlocks = 5;
    superBlock->TotalBlocks = totalBlocks;
    superBlock->Magic = 0x20001208;

    // 4. 将超级块写入磁盘
    DiskCacheWrite(0, (void*)superBlock);
    FreeOnePage(superBlock);

    // 5. 创建根目录
    InodeID id = allocateOneInodeID();
    DiskInode* rootInode = newEmptyDiskInode();
    rootInode->Type = FT_DIRECTORY;
    rootInode->RefCount = 1;
    writeDiskInode(id, rootInode);
    Free(rootInode);

    // 6. 初始化当前目录
    currentDirectoryInodeID = id;
    currentPath = (String)Malloc(256);
    MemoryCopy(currentPath, "/", 1);
}

Boolean ClearFileContentByInodeID(InodeID id) {
    if (!isFile(id)) {
        return FALSE;
    }

    DiskInode* fileInode = readDiskInode(id);
    u32 size = fileInode->Size;
    u32 blocks = (size + 511) / 512; // 向上取整

    if (blocks > INODE_DIRECT_BLOCKS) {
        Panic("[ClearFileContent] indirect inode is not implemented");
    }

    for (u32 i = 0; i < blocks; i++) {
        u32 blockID = fileInode->DirectBlock[i];
        freeOneDataBlockID(blockID);
        fileInode->DirectBlock[i] = 0;
    }
    fileInode->Size = 0;

    writeDiskInode(id, fileInode);
    Free(fileInode);
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
    MemoryCopy(entry->Name, name, StringLength(name));
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

    Panic("allocateOneInodeID: no inode id can be allocated");
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

    Panic("[allocateOneDataBlockID] no data block id can be allocated");
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
    if (StringEqual(name, "")) {
        return 0;
    }

    DiskInode* dirInode = readDiskInode(id);
    if (dirInode->Type != FT_DIRECTORY) {
        Free(dirInode);
        return 0;
    }

    u32 size = dirInode->Size;
    u32 blocks = (size + 511) / 512; // 向上取整
    for (u32 i = 0; i < blocks; i++) {
        u32 blockID = dirInode->DirectBlock[i];
        u8 buffer[512];
        DiskCacheRead(blockID, buffer);
        for (u32 j = 0; j < 512; j += sizeof(DirectoryEntry)) {
            DirectoryEntry* entry = (DirectoryEntry*)(buffer + j);
            if (StringEqual(entry->Name, name)) {
                Free(dirInode);
                return entry->InodeID;
            }
        }
    }

    Free(dirInode);
    return 0;
}

static Boolean removeInodeByNameFromDirectory(InodeID id, String name) {
    DiskInode* dirInode = readDiskInode(id);
    if (dirInode->Type != FT_DIRECTORY) {
        Free(dirInode);
        return FALSE;
    }

    u32 size = dirInode->Size;
    u32 blocks = (size + 511) / 512; // 向上取整
    for (u32 i = 0; i < blocks; i++) {
        u32 blockID = dirInode->DirectBlock[i];
        u8 buffer[512];
        DiskCacheRead(blockID, buffer);
        for (u32 j = 0; j < 512; j += sizeof(DirectoryEntry)) {
            DirectoryEntry* entry = (DirectoryEntry*)(buffer + j);
            if (StringEqual(entry->Name, name)) {
                // 1.清空文件/目录的数据块
                ClearFileContentByInodeID(entry->InodeID);
                // 2. 归还inode id
                freeOneInodeID(entry->InodeID);
                // 3. 删除目录项
                entry->InodeID = 0;
                MemoryFree(entry->Name, 28);
                MemoryCopy(buffer + j, entry, sizeof(DirectoryEntry));
                DiskCacheWrite(blockID, buffer);

                // 4. 更新目录大小
                dirInode->Size -= sizeof(DirectoryEntry);
                writeDiskInode(id, dirInode);

                Free(dirInode);
                return TRUE;
            }
        }
    }

    Free(dirInode);
    return FALSE;
}

static Boolean isDirectory(InodeID id) {
    Boolean result = FALSE;
    DiskInode* diskInode = readDiskInode(id);
    result = diskInode->Type == FT_DIRECTORY;

    Free(diskInode);
    return result;
}

static Boolean isFile(InodeID id) {
    Boolean result = FALSE;
    DiskInode* diskInode = readDiskInode(id);
    result = diskInode->Type == FT_FILE;

    Free(diskInode);
    return result;
}