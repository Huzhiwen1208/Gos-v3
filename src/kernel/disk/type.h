#pragma once
#include "../common/type.h"
#include "../device/type.h"
#include "../ds/type.h"

#define DISK_COUNT_PER_CONTROLLER 2
#define IDE_CONTROLLER_COUNT 2

#define IdeIobaseMaster 0x1F0  // IDE主控制器的IO基址
#define IdeIobaseSlave 0x170  // IDE从控制器的IO基址
#define IdeLbaMaster 0xe0 // IDE主控制器的选择器
#define IdeLbaSlave 0xf0 // IDE从控制器的选择器
#define IdeTimeout 100000 // IDE超时时间
#define IdeDataRegister 0 // 数据寄存器
#define IdeErrorRegister 1 // 错误寄存器
#define IdeFeatureRegister 1 // 特征寄存器
#define IdeSectorCountRegister 2 // 扇区计数寄存器
#define IdeSectorLocationRegister 3 // 扇区位置寄存器
#define IdeCylinderLowRegister 4 // 柱面低位寄存器
#define IdeCylinderHighRegister 5 // 柱面高位寄存器
#define IdeLbaLow 3 // LBA低位寄存器
#define IdeLbaMid 4 // LBA中位寄存器
#define IdeLbaHigh 5 // LBA高位寄存器
#define IdeSelectorRegister 6 // 选择器寄存器
#define IdeStatusRegister 7 // 状态寄存器
#define IdeCommandRegister 7 // 命令寄存器
#define IdeAlternateStatusRegister 0x206 // 备用状态寄存器
#define IdeControlRegister 0x206 // 控制寄存器
#define IdeDriverAddressRegister 0x206 // 驱动器地址寄存器

#define IdeStatusNull 0 // 空状态
#define IdeStatusErr 1  // 错误状态
#define IdeStatusIdx 2 // 索引状态
#define IdeStatusCorr 4 // 校正状态
#define IdeStatusDrq 8 // 数据请求状态
#define IdeStatusDsc 16 // 驱动器状态
#define IdeStatusDwf 32 // 驱动器准备好状态
#define IdeStatusDrdy 64 // 驱动器准备好状态
#define IdeStatusBusy 128 // 忙状态
#define IdeCommandRead 0x20 // 读命令
#define IdeCommandWrite 0x30 // 写命令
#define IdeCommandIdentify 0xEC // 识别命令
#define IdeCommandDiagnostic 0x90 // 诊断命令

typedef struct IdeDisk {
    char Name[NAME_LENGTH]; // 磁盘名称
    struct IdeController* Controller; // 磁盘所属IDE控制器
    u32 Selector; // 磁盘的选择器
    Boolean IsMaster; // 是否是主盘
    Size TotalSector; // 该磁盘总扇区数
    u32 C, H, S; // 柱面数、磁头数、扇区数
} IdeDisk;

typedef struct IdeController {
    char Name[NAME_LENGTH]; // IDE控制器名称
    ReentrantLock* rlock; // 可重入互斥锁
    u32 IoBase; // IO基址
    IdeDisk DiskList[DISK_COUNT_PER_CONTROLLER]; // IDE控制器下的磁盘列表
    IdeDisk* CurrentDisk; // 当前磁盘
    u8 ControlByte; // 控制字节
} IdeController;

// IDE 读取参数
struct IdeParam {
    u16 Config;
    u16 Cylinders;
    u16 unused_;
    u16 Heads;
    u16 unused_1[5 - 3];
    u16 Sectors;
    u16 unused_2[9 - 6];
    u8 Serial[20];
    u16 unused_3[22 - 19];
    u8 Firmware[8];
    u8 Model[40];
    u8 DrqSectors;
    u8 unused_4[3];
    u16 Capabilities;
    u16 unused_5[59 - 49];
    u32 TotalSectors;
    u16 unused_6;
    u16 MDMAMode;
    u8 unused_7;
    u8 PioMode;
    u16 unused_8[79 - 64];
    u16 MajorVersion;
    u16 MinorVersion;
    u16 CommmandSets[87 - 81];
    u16 unused_9[118 - 87];
    u16 SupportSettings;
    u16 EnableSettings;
    u16 unused_10[221 - 120];
    u16 TransportMajor;
    u16 TransportMinor;
    u16 unused_11[254 - 223];
    u16 Integrity;
} StructNotAlign;
typedef struct IdeParam IdeParam;

typedef enum DeviceID {
    GOS_DEVICE,
    FS_DEVICE
} DeviceID;