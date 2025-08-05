# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

# 项目说明
这是一个Windows内核驱动ARK项目，包含Ring-0内核驱动(NTDriver)和Ring-3用户模式GUI应用(ARK)两个主要组件，用于系统监控和反Rootkit分析。

# 身份设置
用户是正在学习windows内核的学生，你除了生产代码，更重要的是整理思维链路，数据流动，告诉用户为什么这么做。

当完成一段功能后，自动生成对应的md笔记，保存用户不懂的知识

# 代码架构

## 项目结构
```
NTDriver/          # 内核驱动组件 (Ring-0)
├── driver.cpp/h   # 主驱动入口点和IRP调度
├── process.cpp/h  # 进程枚举和管理
├── module.cpp/h   # 模块枚举
├── callback.cpp/h # 回调机制分析
└── ssdt.cpp/h     # 系统服务描述符表分析

ARK/               # 用户模式GUI应用 (Ring-3)  
├── main.cpp       # DirectX 9应用入口
├── App.cpp/h      # 主应用类
├── ProcessWnd.h   # 进程窗口UI
├── ModuleWnd.h    # 模块窗口UI
├── CallbackWnd.h  # 回调窗口UI
└── ArkR3.cpp/h    # Ring-3驱动通信层

include/proto.h    # 驱动与应用通信协议
```

## 通信机制
- 设备名称: `\\Device\\ADriver1`
- 符号链接: `\\DosDevices\\ADriver1`
- 通信方式: DeviceIoControl + 自定义IOCTL码
- 主要数据结构: PROCESS_INFO, MODULE_INFO, CALLBACK_INFO, SSDT_INFO

# 代码风格
- 遵循用户的编程风格
- 重点解释Windows内核概念和数据流动
- 在每次写入文件之前，必须检测你输出的代码和当前页的字符集设置是否一致！你总是给我设置错误的字符集
- 小功能 先实现R0 再实现通讯 最后实现R1

# 通讯码宏对应关系

## 已实现功能

| 功能模块 | 通讯码宏 | 驱动端函数 | 用户态接口 | 状态 |
|---------|---------|----------|----------|------|
| **内存操作** |
| 读内存 | `CTL_READ_MEM` | - | - | ✗ |
| 写内存 | `CTL_WRITE_MEM` | - | - | ✗ |
| 附加进程读 | `CTL_ATTACH_MEM_READ` | - | - | ✗ |
| 附加进程写 | `CTL_ATTACH_MEM_WRITE` | - | - | ✗ |
| **GDT表** |
| 获取GDT数据 | `CTL_GET_GDT_DATA` | `DriverBase.cpp:110` | - | ✓ |
| **进程管理** |
| 进程数量 | `CTL_ENUM_PROCESS_COUNT` | `DriverBase.cpp:138` | - | ✓ |
| 进程枚举 | `CTL_ENUM_PROCESS` | `DriverBase.cpp:157` | - | ✓ |
| 终止进程 | `CTL_KILL_PROCESS` | - | - | ✗ |
| 强制终止 | `CTL_FORCE_KILL_PROCESS` | `DriverBase.cpp:325` | - | ✓ |
| **模块管理** |
| 模块数量 | `CTL_ENUM_MODULE_COUNT` | `DriverBase.cpp:174` | - | ✓ |
| 模块枚举 | `CTL_ENUM_MODULE` | `DriverBase.cpp:192` | - | ✓ |
| 进程模块数量 | `CTL_ENUM_PROCESS_MODULE_COUNT` | - | - | ✗ |
| 进程模块枚举 | `CTL_ENUM_PROCESS_MODULE` | - | - | ✗ |
| **SSDT** |
| SSDT枚举 | `CTL_ENUM_SSDT` | `DriverBase.cpp:209` | - | ✓ |
| 开始监控 | `CTL_START_SSDTHOOK` | - | - | ✗ |
| 结束监控 | `CTL_END_SSDTHOOK` | - | - | ✗ |
| **系统回调** |
| 回调枚举 | `CTL_ENUM_CALLBACK` | `DriverBase.cpp:228` | - | ✓ |
| 删除回调 | `CTL_DELETE_CALLBACK` | `DriverBase.cpp:251` | - | ✓ |
| **驱动分析** |
| 驱动数量 | `CTL_ENUM_DRIVER_COUNT` | - | - | ✗ |
| 驱动枚举 | `CTL_ENUM_DRIVER` | - | - | ✗ |
| 派遣Hook | `CTL_ENUM_DISPATCH_HOOK` | `DriverBase.cpp:270` | - | ✓ |
| 设备栈 | `CTL_ENUM_DEVICE_STACK` | `DriverBase.cpp:289` | - | ✓ |
| **网络监控** |
| 网络端口 | `CTL_ENUM_NETWORK_PORT` | `DriverBase.cpp:307` | - | ✓ |
| **文件操作** |
| 解锁文件 | `CTL_UNLOCK_FILE` | `DriverBase.cpp:336` | - | ✓ |
| 强制删除 | `CTL_FORCE_DELETE_FILE` | `DriverBase.cpp:353` | - | ✓ |
| **其他** |
| 设置PDB路径 | `CTL_SET_PDB_PATH` | `DriverBase.cpp:91` | `ArkR3.cpp:49` | ✓ |
| 窗口透明度 | `CTL_SET_WINDOW_TRANSPARENCY` | - | - | ✗ |

## 待实现功能优先级

### HIGH 优先级
1. **进程强制结束**: `CTL_KILL_PROCESS` - 需实现驱动端和用户态接口  
2. **内存读写**: `CTL_READ_MEM`, `CTL_WRITE_MEM` - 跨进程内存操作核心功能
3. **SSDT恢复**: 新增恢复被HOOK系统调用的通讯码
4. **回调恢复**: 新增恢复被HOOK回调的通讯码

### MEDIUM 优先级  
5. **隐藏进程检测**: 新增检测隐藏进程的通讯码
6. **隐藏模块检测**: 新增检测隐藏模块的通讯码
7. **对象回调**: 新增对象回调枚举的通讯码
8. **ShadowSSDT**: 新增Win32k系统调用表相关通讯码
9. **IDT中断表**: 新增中断表枚举和恢复的通讯码

# 项目需求

**1.** **进程管理**

1) 遍历(已完成) - `CTL_ENUM_PROCESS_COUNT` + `CTL_ENUM_PROCESS`
2) 强制结束 - `CTL_FORCE_KILL_PROCESS` (已实现) + `CTL_KILL_PROCESS` (待实现)
3) 检查隐藏(TODO) - 需新增通讯码

**2.** **驱动模块**

1) 遍历(已完成) - `CTL_ENUM_MODULE_COUNT` + `CTL_ENUM_MODULE`
2) 检查隐藏（TODO） - 需新增通讯码

**3.** **系统回调**

1) 遍历（进程、线程、模块、对象）- `CTL_ENUM_CALLBACK` (进程、线程、模块已完成，对象TODO)
2) 恢复() - 需新增通讯码

**4.** **SSDT**

1) 遍历 （已完成）- `CTL_ENUM_SSDT`
2) 恢复 - 需新增通讯码

**5.** **ShadowSSDT**

1) 遍历 - 需新增通讯码
2) 恢复 - 需新增通讯码

**6.** **中断表**

1) 遍历 - 需新增通讯码
2) 恢复 - 需新增通讯码

**7.** **GDT**表

1) 遍历 （已完成）- `CTL_GET_GDT_DATA`

**8.** **读写进程内存**

1) 读取指定进程内存 - `CTL_READ_MEM` + `CTL_ATTACH_MEM_READ` (待实现)
2) 写入指定进程内存 - `CTL_WRITE_MEM` + `CTL_ATTACH_MEM_WRITE` (待实现)

# 需求详细实现对应表

| 需求分类 | 具体功能 | 通讯码宏 | R0驱动端函数 | R3用户态函数 | 数据结构 | 状态 |
|---------|---------|---------|-------------|-------------|---------|------|
| **1. 进程管理** |
| 进程枚举 | 遍历所有进程 | `CTL_ENUM_PROCESS_COUNT`<br>`CTL_ENUM_PROCESS` | `EnumProcessFromLinksEx()`<br>`process.cpp:32` | `ArkR3::GetProcessList()` | `PROCESS_INFO` | ✓ |
| 进程终止 (普通) | 正常结束进程 | `CTL_KILL_PROCESS` | **待实现** | **待实现** | `HANDLE ProcessId` | ✗ |
| 进程终止 (强制) | 强制结束进程 | `CTL_FORCE_KILL_PROCESS` | `TerminateProcessByApi()`<br>`process.cpp:80`<br>`TerminateProcessByThread()`<br>`process.cpp:112` | `ArkR3::ProcessForceKill()`<br>`ArkR3.cpp:339` | `ULONG ProcessId` | ✓ |
| 隐藏进程检测 | 检测被隐藏的进程 | **待新增** | **待实现** | **待实现** | **待设计** | ✗ |
| **2. 模块管理** |
| 系统模块枚举 | 枚举系统驱动模块 | `CTL_ENUM_MODULE_COUNT`<br>`CTL_ENUM_MODULE` | `EnumModuleEx()`<br>`module.cpp:14`<br>`EnumModule()`<br>`module.cpp:99` | `ArkR3::GetModuleList()` | `MODULE_INFO` | ✓ |
| 进程模块枚举 | 枚举指定进程模块 | `CTL_ENUM_PROCESS_MODULE_COUNT`<br>`CTL_ENUM_PROCESS_MODULE` | **待实现** | **待实现** | `PROCESS_MODULE_REQ` | ✗ |
| 隐藏模块检测 | 检测被隐藏的模块 | **待新增** | **待实现** | **待实现** | **待设计** | ✗ |
| **3. 系统回调** |
| 进程回调 | 进程创建/终止回调 | `CTL_ENUM_CALLBACK` | `EnumCallbacks()`<br>`callback.cpp:45`<br>type=`TypeProcess` | `ArkR3::GetCallbackList()` | `CALLBACK_INFO` | ✓ |
| 线程回调 | 线程创建/终止回调 | `CTL_ENUM_CALLBACK` | `EnumCallbacks()`<br>type=`TypeThread` | `ArkR3::GetCallbackList()` | `CALLBACK_INFO` | ✓ |
| 模块加载回调 | 模块/映像加载回调 | `CTL_ENUM_CALLBACK` | `EnumCallbacks()`<br>type=`TypeImage` | `ArkR3::GetCallbackList()` | `CALLBACK_INFO` | ✓ |
| 对象回调 | 对象操作回调 | `CTL_ENUM_CALLBACK` | `EnumCallbacks()`<br>type=`TypeObject` | `ArkR3::GetCallbackList()` | `CALLBACK_INFO` | ✗ |
| 回调删除/恢复 | 删除被HOOK的回调 | `CTL_DELETE_CALLBACK` | `DeleteCallback()`<br>`callback.cpp` | `ArkR3::DeleteCallback()` | `CALLBACK_DELETE_REQ` | ✓ |
| **4. SSDT系统调用** |
| SSDT枚举 | 枚举系统调用表 | `CTL_ENUM_SSDT` | `EnumSSDT()`<br>`ssdt.cpp:20` | `ArkR3::GetSSDTList()` | `SSDT_INFO` | ✓ |
| SSDT监控 | 开始/结束监控 | `CTL_START_SSDTHOOK`<br>`CTL_END_SSDTHOOK` | **待实现** | **待实现** | `HOOK_SSDT_Index` | ✗ |
| SSDT恢复 | 恢复被HOOK的调用 | **待新增** | **待实现** | **待实现** | **待设计** | ✗ |
| **5. ShadowSSDT** |
| Win32k枚举 | 枚举Win32k调用表 | **待新增** | **待实现** | **待实现** | **待设计** | ✗ |
| Win32k恢复 | 恢复Win32k HOOK | **待新增** | **待实现** | **待实现** | **待设计** | ✗ |
| **6. 中断表IDT** |
| IDT枚举 | 枚举中断描述符表 | **待新增** | **待实现** | **待实现** | **待设计** | ✗ |
| IDT恢复 | 恢复被HOOK的中断 | **待新增** | **待实现** | **待实现** | **待设计** | ✗ |
| **7. GDT全局描述符** |
| GDT枚举 | 枚举全局描述符表 | `CTL_GET_GDT_DATA` | `GetGDTData()`<br>`DriverBase.cpp:110` | `ArkR3::GetGDTData()` | `GDT_DATA_REQ`<br>`SegmentDescriptor` | ✓ |
| **8. 内存操作** |
| 直接内存读 | 读取内核/进程内存 | `CTL_READ_MEM` | **待实现** | **待实现** | `KERNEL_RW_REQ` | ✗ |
| 直接内存写 | 写入内核/进程内存 | `CTL_WRITE_MEM` | **待实现** | **待实现** | `KERNEL_RW_REQ` | ✗ |
| 附加进程读 | 附加到进程读内存 | `CTL_ATTACH_MEM_READ` | **待实现** | **待实现** | `PROCESS_MEM_REQ` | ✗ |
| 附加进程写 | 附加到进程写内存 | `CTL_ATTACH_MEM_WRITE` | **待实现** | **待实现** | `PROCESS_MEM_REQ` | ✗ |
| **9. 驱动分析** |
| 驱动枚举 | 枚举系统驱动对象 | `CTL_ENUM_DRIVER_COUNT`<br>`CTL_ENUM_DRIVER` | **待实现** | **待实现** | **待设计** | ✗ |
| 派遣函数Hook | 检测IRP派遣Hook | `CTL_ENUM_DISPATCH_HOOK` | `EnumDrvMJHooked()`<br>`driver.cpp:246` | **待实现** | `DISPATCH_HOOK_INFO` | ✓ |
| 设备栈分析 | 分析过滤驱动栈 | `CTL_ENUM_DEVICE_STACK` | `EnumDeviceStackAttach()`<br>`driver.cpp:9` | **待实现** | `DEVICE_STACK_INFO` | ✓ |
| **10. 网络监控** |
| 网络端口 | 枚举网络连接 | `CTL_ENUM_NETWORK_PORT` | `EnumNetworkPort()`<br>`network.cpp:4` | **R3实现** | `NETWORK_PORT_INFO` | ✓ |
| **11. 文件操作** |
| 文件解锁 | 解锁占用文件 | `CTL_UNLOCK_FILE` | `UnlockFile()`<br>`file.cpp` | **待实现** | `FILE_REQ` | ✓ |
| 文件强删 | 强制删除文件 | `CTL_FORCE_DELETE_FILE` | `ForceDeleteFile()`<br>`file.cpp` | **待实现** | `FILE_REQ` | ✓ |

## 优先实现顺序

### HIGH优先级 (核心反Rootkit功能)
1. **CTL_KILL_PROCESS** - 普通进程终止 (`process.cpp` + `ArkR3.cpp`)
2. **CTL_READ_MEM/CTL_WRITE_MEM** - 直接内存读写 (新增到`DriverBase.cpp`)  
3. **CTL_ATTACH_MEM_READ/WRITE** - 进程内存读写 (新增到`DriverBase.cpp`)
4. **SSDT恢复功能** - 新增通讯码和实现 (`ssdt.cpp` + `ArkR3.cpp`)

### MEDIUM优先级 (检测扩展功能)  
5. **隐藏进程检测** - 对比多种枚举方式差异 (`process.cpp`)
6. **隐藏模块检测** - 对比PEB与系统枚举差异 (`module.cpp`)  
7. **对象回调枚举** - 完善TypeObject支持 (`callback.cpp`)
8. **ShadowSSDT** - Win32k系统调用表分析 (新增`shadowssdt.cpp`)
9. **IDT中断表** - 中断描述符表分析 (新增`idt.cpp`)
