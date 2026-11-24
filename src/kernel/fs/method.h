#pragma once
#include "../common/type.h"
#include "type.h"
#include "../common/file_op.h"

void InitializeFileSystem();

String GetCurrentPath();
// pwd命令
void PrintfWorkingDirectory();
// 清空文件内容
Boolean ClearFileContent(String path);
// 读取指定文件第n行的内容到buf中去，n从1开始，如果n超过文件行数，返回FALSE
Boolean ReadFileLine(String path, u32 n, String buf);
// write命令，将内容写入文件，要求文件必须存在
Boolean WriteFileContent(String path, String content, Boolean trunc);
// cat命令，打印文件内容 cat -n 5 path指打印文件前n行
Boolean PrintFileContent(String path, u32 n);
// cd命令，切换当前目录到path下，如果path不存在或者不是目录，则不切换并抛出错误信息返回FALSE
Boolean ChangeDirectory(String path);
// ls [-a] [path]，支持绝对、相对路径以及 . 和 ..
void ListFiles(String option, String path);
// 仅供内核配置和测试使用；当前为单用户 owner 权限模型。
Boolean SetFilePermissions(String path, u16 mode);
// mkdir [-p] path，支持绝对、相对路径以及 . 和 ..
Boolean MakeDirectory(String path, String option);
/// @brief 在inodeID指定的目录下创建文件，文件名为name，文件类型为type，如果id==-1, 那么就在当前目录下创建文件
/// @param id 目录的InodeID
/// @param name 文件名
/// @param type 文件类型
/// @return 创建成功返回创建文件的InodeID，否则返回-1
InodeID CreateFile(InodeID id, String name, FileType type);
// rm命令，删除指定文件或者目录，如果是目录，那么需要加上-r选项
// rm [-r] /x/y/z    rm [-r] ./x/y/* 
// -r支持递归，完整标志由RemovePath处理。
Boolean RemoveFile(String option, String path);
Boolean ClearFileContentByInodeID(InodeID id);
// 原inode/EOF模型上的文件操作；失败时打印对应命令错误。
Boolean RemovePath(String path, u32 flags);
Boolean CopyPath(String source, String target, u32 flags);
Boolean MovePath(String source, String target, u32 flags);
