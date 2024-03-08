# GOS 操作系统开发
## 介绍
这是一个基于虚拟的裸机硬件出发，设计开发的操作系统。从汇编手写MBR开始，一直到最后呈现出一个类Linux系统的GOS操作系统。其中，我们实现了操作系统经典资源管理组件：
- 显存、显示器的管理（此处采用的是阴极射线管设备）。
- 伙伴算法实现的堆内存分配
- 进程调度
- 中断、异常、系统调用、特权级
- 页式虚拟内存
- 磁盘、键盘等IO设备的管理
- Inode类型的文件系统设计
- GShell终端控制台
等等内容

## GOS架构图
TODO

## 代码分布与链接
总目录链接：[开发目录文档索引](https://m13n4gzucg.feishu.cn/docx/Fth3d2wnAoOiNIxilF3cxOnpnwg)
| 分支         | 分支内容                      | 对应文档链接                                                                                               |
| ------------ | ----------------------------- | ---------------------------------------------------------------------------------------------------------- |
| master       | 操作系统最终版本              | 该README                                                                                                   |
| feature/ch0  | Bootloader，进入内核          |                                                                                                            |
| feature/ch1  | 控制台输出                    |                                                                                                            |
| feature/ch2  | 输出扩展+内存初步管理         |                                                                                                            |
| feature/ch3  | 中断异常与系统调用            |                                                                                                            |
| feature/ch4  | 进程管理与调度                |                                                                                                            |
| feature/ch5  | 用户态与内核态                |                                                                                                            |
| feature/ch6  | 页式虚拟存储系统                |                                                                                                            |
| feature/lab0 | 完成中间代码的磁盘加载和执行  | [Location](https://m13n4gzucg.feishu.cn/docx/EMypdsUgSo63qoxcbN2cXXi2nrc#part-Pn6Odp6R0oGDMlxg6U9cJqj2nzb) |
| feature/lab1 | log日志实现与固定分区分配     | [Location](https://m13n4gzucg.feishu.cn/docx/ARyod2E3oortWqxzDVgcDisyndg#part-GQCIdkCMTogsdxxahMic458wnNe) |
| feature/lab2 | 中断与系统调用                | [Location](https://m13n4gzucg.feishu.cn/docx/JpE1dhEDdod47wxpl55cqiUAnBb#part-VPSkd7m0DohpEZxDJ17cusubnng) |
| feature/lab3 | FCFS进程调度BUG修复与调度实现 | [Location](https://m13n4gzucg.feishu.cn/docx/HTwvdVZqHoYKsnx2uRQcnhiTnbc#part-D5VddklOEoGFnjxT4pWcQEppnwb) |
| feature/lab4 | 系统调用printf实现            | [Location](https://m13n4gzucg.feishu.cn/docx/HPjrdsR2xooOTRxFsTlc40EBn7y#part-GcGldd4W2owgaTxJ7YxcsXNtngb) |
| feature/lab5 | 缺页异常实现            | [Location](https://m13n4gzucg.feishu.cn/docx/FrovdSQMOoYlZpxfUT9c5ai3nSd#part-WlRZd88uQoUYnoxbk6icn4Fvnqh) |
****