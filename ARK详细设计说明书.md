# ARK（Anti-Rootkit）详细设计说明书

## 1 引言

### 1.1 编写目的
本详细设计说明书的目的是为ARK（Anti-Rootkit）程序提供详细的设计说明，描述其内部结构、功能模块实现和工作原理。该文档将为开发人员、维护人员以及学习Windows内核编程的人员提供深入的技术参考。

### 1.2 背景
该程序是一个基于Windows内核驱动技术实现的反Rootkit工具，采用Ring-0内核驱动和Ring-3用户态GUI应用的双层架构。程序能够检测和分析系统中的恶意代码、隐藏进程、隐藏驱动、Hook检测等高级威胁，支持SSDT恢复、IDT恢复、ShadowSSDT恢复等内核级反Rootkit功能。

### 1.3 参考资料
- Windows Driver Kit (WDK) 文档
- Windows PE文件格式规范
- Intel手册

## 2 程序系统的结构

### 2.1 系统架构概览
```
┌───────────────────────────────────────┐    DeviceIoControl    ┌────────────────────────────────────────┐
│           ARK GUI (Ring-3)            │ ◄──────────────────► │        NTDriver (Ring-0)               │
├───────────────────────────────────────┤                       ├────────────────────────────────────────┤
│ 应用程序框架层                          │                       │ 驱动管理层                              │
│ ├─ main.cpp          - 程序入口        │                       │ ├─ driver.cpp        - 驱动入口/IRP     │
│ ├─ App.cpp/h         - 应用主控制器     │                       │ ├─ DriverBase.cpp/h  - 核心IOCTL调度   │
│ └─ driverloader.cpp/h - 驱动加载管理    │                       │ └─ mydef.h           - 全局定义        │
├───────────────────────────────────────┤                       ├────────────────────────────────────────┤
│ 通信接口层                             │                       │ 功能实现层                              │
│ ├─ ArkR3.cpp/h       - 驱动通信封装    │                       │ ├─ process.cpp/h     - 进程管理        │
│ ├─ interface.h       - 数据结构定义    │                       │ ├─ module.cpp/h      - 模块分析        │
│ └─ ezpdb.hpp/lib     - PDB符号解析     │                       │ ├─ callback.cpp/h    - 回调监控        │
├───────────────────────────────────────┤                       │ ├─ ssdt.cpp/h        - 系统调用表      │
│ 用户界面层                             │                       │ ├─ file.cpp/h        - 文件操作        │
│ ├─ MenuBar.cpp/h     - 主菜单栏        │                       │ └─ network.cpp/h     - 网络监控        │
│ ├─ ProcessWnd.cpp/h  - 进程窗口        │                       ├────────────────────────────────────────┤
│ ├─ ModuleWnd.cpp/h   - 模块窗口        │                       │ PDB符号解析层                           │
│ ├─ CallbackWnd.cpp/h - 回调窗口        │                       │ ├─ Pdber.hpp         - PDB解析器       │
│ ├─ KernelWnd.cpp/h   - 内核窗口        │                       │ ├─ oxygenPdb.h       - 符号处理        │
│ ├─ FileWnd.cpp/h     - 文件窗口        │                       │ └─ oxygenpdb.lib/pdb - PDB库文件       │
│ ├─ NetworkWnd.cpp/h  - 网络窗口        │                       ├────────────────────────────────────────┤
│ ├─ HookWnd.cpp/h     - Hook分析窗口    │                       │ 辅助工具层                              │
│ └─ LogWnd.cpp/h      - 日志窗口        │                       │ ├─ pe.h              - PE文件解析      │
├───────────────────────────────────────┤                       │ ├─ networkdef.h      - 网络定义        │
│ 工具组件层                             │                       │ └─ driver_old.cpp    - 旧版本兼容      │
│ ├─ FileManager.cpp/h - 文件管理器      │                       └────────────────────────────────────────┘
│ ├─ imfilebrowser.h   - 文件浏览对话框   │                                                                
│ ├─ imgui_*.h         - ImGui界面组件   │                                                                
│ ├─ imguiLog.h        - 日志组件        │            ┌─────────────────────────────┐                 
│ ├─ imgui_momory_editor.h - 内存编辑器  │            │      include/proto.h       │                 
│ ├─ pathtrans.h       - 路径转换工具    │            │   (通信协议数据结构定义)      │                 
│ └─ WinsockHeader.h   - 网络头文件      │            │                             │                 
└───────────────────────────────────────┘            │ ├─ PROCESS_INFO             │                 
                                                     │ ├─ MODULE_INFO              │                 
生成文件：ARK/x64/Debug/ADriver1.sys                  │ ├─ CALLBACK_INFO            │                 
└─ 编译后的内核驱动文件                                │ ├─ SSDT_INFO                │                 
                                                     │ ├─ IDT_INFO                 │                 
                                                     │ └─ 各种控制码定义             │                 
                                                     └─────────────────────────────┘                 
```

### 2.2 主要功能模块

#### 2.2.1 内核驱动层 (Ring-0) - NTDriver目录
1. **驱动管理模块** (`driver.cpp/h`)：
   - 驱动程序入口点(DriverEntry)和卸载(DriverUnload)
   - IRP(I/O Request Packet)调度处理
   - 设备对象创建和符号链接管理

2. **核心调度模块** (`DriverBase.cpp/h`)：
   - 40+个IOCTL控制码的统一调度
   - 输入输出缓冲区管理和数据传输
   - 错误处理和状态返回

3. **进程监控模块** (`process.cpp/h`)：
   - ActiveProcessLinks链表遍历和EPROCESS结构内存搜索
   - 隐藏进程检测算法和进程强制终止功能

4. **模块分析模块** (`module.cpp/h`)：
   - PsLoadedModuleList驱动枚举和PE头特征内存搜索
   - 隐藏驱动检测算法和模块完整性验证

5. **系统调用监控** (`ssdt.cpp/h`)：
   - SSDT/ShadowSSDT枚举，Hook检测与恢复算法
   - ntdll.dll/win32u.dll PE解析，IDT异常向量恢复

6. **回调机制分析** (`callback.cpp/h`)：
   - 进程/线程/模块/对象四类回调完整枚举
   - 回调删除和恢复操作

7. **文件网络模块** (`file.cpp/h`, `network.cpp/h`)：
   - 文件句柄强制解锁和文件强制删除
   - TCP/UDP端口枚举和网络连接分析

8. **PDB符号解析** (`Pdber.hpp`, `oxygenPdb.h/.lib`)：
   - ntoskrnl.exe和win32k.sys符号加载
   - 函数地址RVA计算和调试符号缓存

#### 2.2.2 用户态应用层 (Ring-3) - ARK目录

1. **应用程序框架层**：
   - `main.cpp`：程序入口，DirectX9初始化，消息循环
   - `App.cpp/h`：应用主控制器，窗口管理，界面渲染协调
   - `driverloader.cpp/h`：驱动自动加载，服务管理，权限提升

2. **通信接口层**：
   - `ArkR3.cpp/h`：驱动通信封装，IOCTL调用管理，数据转换
   - `interface.h`：通信接口定义和函数声明
   - `ezpdb.hpp/lib`：用户态PDB符号解析库

3. **用户界面层**：
   - `MenuBar.cpp/h`：主菜单栏，功能入口，设置管理
   - `ProcessWnd.cpp/h`：进程窗口，进程列表，操作按钮
   - `ModuleWnd.cpp/h`：模块窗口，驱动列表，隐藏检测
   - `CallbackWnd.cpp/h`：回调窗口，四类回调展示
   - `KernelWnd.cpp/h`：内核窗口，SSDT/IDT/GDT分析
   - `FileWnd.cpp/h`：文件窗口，文件操作功能
   - `NetworkWnd.cpp/h`：网络窗口，端口连接监控
   - `HookWnd.cpp/h`：Hook分析窗口，Hook检测结果
   - `LogWnd.cpp/h`：日志窗口，操作记录和调试信息

4. **工具组件层**：
   - `FileManager.cpp/h`：文件管理器，PDB路径配置
   - `imfilebrowser.h`：文件浏览对话框组件
   - `imgui_*.h`：ImGui界面渲染组件
   - `imgui_momory_editor.h`：内存编辑器组件
   - `pathtrans.h`：路径转换和字符编码工具

#### 2.2.3 共享协议层 - include目录
- `proto.h`：通信协议数据结构定义，IOCTL控制码，共享常量定义

## 3 程序（ARK）设计说明

### 3.1 程序描述
ARK是一个基于DirectX 9和ImGui界面的反Rootkit工具，采用内核驱动与用户态应用分离的架构设计。程序通过DeviceIoControl与内核驱动通信，能够进行深度的系统分析和恶意代码检测。系统具备完整的SSDT/ShadowSSDT/IDT恢复能力，支持隐藏进程和驱动检测，提供直观的图形化界面展示分析结果。

### 3.2 功能

#### 3.2.1 进程管理功能
- **进程枚举**：通过ActiveProcessLinks链表遍历系统进程
- **隐藏进程检测**：内存搜索EPROCESS结构，对比链表枚举结果
- **进程强制终止**：通过API和线程终止两种方式强制结束进程
- **进程详细信息**：PID、PPID、路径、创建时间等完整信息

#### 3.2.2 模块分析功能
- **驱动模块枚举**：遍历PsLoadedModuleList获取系统驱动列表
- **隐藏驱动检测**：内存搜索PE头特征，发现未在系统链表中的驱动
- **模块完整性验证**：PE头解析、数字签名验证、路径分析

#### 3.2.3 系统调用监控
- **SSDT分析**：枚举系统服务描述符表，检测Hook
- **SSDT恢复**：通过PDB符号解析原始地址，修复被Hook的系统调用
- **ShadowSSDT分析**：Win32k系统调用表分析，NtUser*/NtGdi*函数监控
- **ShadowSSDT恢复**：解析win32u.dll获取调用映射，恢复图形子系统Hook

#### 3.2.4 系统回调分析
- **回调枚举**：进程、线程、模块、对象四类系统回调完整枚举
- **回调删除**：通过ObUnRegisterCallbacks删除恶意回调注册
- **回调完整性验证**：模块路径分析、数字签名验证

#### 3.2.5 中断表管理
- **IDT枚举**：中断描述符表完整枚举，异常向量分析
- **IDT恢复**：恢复0x00-0x13异常向量的Hook，修复系统完整性

#### 3.2.6 系统表分析
- **GDT分析**：全局描述符表枚举，段描述符解析
- **内存搜索**：内核地址空间扫描，特征匹配算法

### 3.3 性能指标
1. **进程枚举性能**：支持1000+进程的快速枚举
2. **驱动检测效率**：256MB内核地址空间扫描，4KB页对齐优化
3. **系统调用覆盖**：SSDT 460+函数，ShadowSSDT 1000+函数完整监控
4. **回调监控范围**：覆盖所有对象类型，100+回调函数检测
5. **内存搜索精度**：16字节对齐EPROCESS搜索，多重验证算法
6. **实时更新频率**：1秒刷新间隔，支持实时监控

### 3.4 输入项
1. **用户操作输入**：鼠标点击、键盘输入、菜单选择
2. **配置文件输入**：PDB路径配置、日志记录设置
3. **系统状态输入**：内核数据结构、系统调用表、进程信息
4. **驱动通信输入**：DeviceIoControl返回的内核数据

### 3.5 输出项
1. **进程信息表格**：PID、进程名、路径、父进程、状态标记
2. **模块信息表格**：基址、大小、路径、版本信息、签名状态
3. **系统调用表格**：序号、函数名、当前地址、原始地址、Hook状态
4. **回调信息表格**：类型、地址、模块、海拔值、状态
5. **系统表信息**：IDT项、GDT项、描述符详情

### 3.6 数据结构相关说明

#### 3.6.1 通信协议结构

**设备名称和控制码定义**
```cpp
// 设备名称定义
#define DEVICE_NAME L"\\Device\\ADriver1"
#define SYMBOL_NAME L"\\DosDevices\\ADriver1" 
#define DOS_NAME    L"\\\\\\.\\ADriver1"

// IOCTL控制码生成宏
#define MY_CTL_CODE(code) CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800 + code, METHOD_BUFFERED, FILE_ANY_ACCESS)

// 主要控制码定义
#define CTL_READ_MEM                MY_CTL_CODE(0)
#define CTL_WRITE_MEM               MY_CTL_CODE(1)
#define CTL_ATTACH_MEM_READ         MY_CTL_CODE(2)      //附加进程读       后续换成API读写了
#define CTL_ATTACH_MEM_WRITE        MY_CTL_CODE(3)      //写
#define CTL_GET_GDT_DATA            MY_CTL_CODE(10)     //获取GDT表数据
#define CTL_ENUM_IDT_COUNT          MY_CTL_CODE(11)     //获取IDT表项数量
#define CTL_ENUM_IDT                MY_CTL_CODE(12)     //获取IDT表数据
#define CTL_ENUM_PROCESS_COUNT      MY_CTL_CODE(20)     // 枚举进程 返回数量
#define CTL_ENUM_PROCESS            MY_CTL_CODE(21)     // 返回数据
#define CTL_KILL_PROCESS            MY_CTL_CODE(22)     // 终止进程
#define CTL_FORCE_KILL_PROCESS      MY_CTL_CODE(24)     // 强制终止进程
#define CTL_MEMSEARCH_PROCESS       MY_CTL_CODE(25)     // 内存特征EPROCESS
#define CTL_ENUM_MODULE_COUNT       MY_CTL_CODE(30)     // 枚举模块 返回数量
#define CTL_ENUM_MODULE             MY_CTL_CODE(31)     // 枚举模块 返回数据
#define CTL_ENUM_PROCESS_MODULE_COUNT MY_CTL_CODE(32)   // 枚举进程模块 返回数量
#define CTL_ENUM_PROCESS_MODULE       MY_CTL_CODE(33)   // 枚举进程模块 返回数据
#define CTL_ENUM_SSDT               MY_CTL_CODE(35)     // 枚举SSDT 返回数据  假定max 500条记录 不要返回数量了
#define CTL_ENUM_ShadowSSDT         MY_CTL_CODE(36)     // 枚举SSDT 返回数据  假定max 3000条记录 不要返回数量了
#define CTL_START_SSDTHOOK          MY_CTL_CODE(40)     //开始监控 R3发ssdt的index  R0返回映射到R3的内存地址
#define CTL_END_SSDTHOOK            MY_CTL_CODE(41)     //结束监控 R3发ssdt的index  
#define CTL_SET_PDB_PATH            MY_CTL_CODE(50)     // 设置PDB下载路径
#define CTL_ENUM_CALLBACK           MY_CTL_CODE(81)     // 枚举回调信息
#define CTL_DELETE_CALLBACK         MY_CTL_CODE(82)     // 删除回调
#define CTL_ENUM_DRIVER_OBJECT      MY_CTL_CODE(91)     // 枚举驱动对象 返回数据
#define CTL_ENUM_DISPATCH_HOOK      MY_CTL_CODE(92)     // 枚举派遣函数Hook
#define CTL_ENUM_DEVICE_STACK       MY_CTL_CODE(93)     // 枚举设备栈分析
#define CTL_ENUM_NETWORK_PORT       MY_CTL_CODE(94)     // 枚举网络连接
#define CTL_UNLOCK_FILE             MY_CTL_CODE(100)            // 解锁文件
#define CTL_FORCE_DELETE_FILE              MY_CTL_CODE(101)     // 粉碎文件
#define CTL_SET_WINDOW_TRANSPARENCY        MY_CTL_CODE(110)     // 设置窗口透明度
#define CTL_RESTORE_SSDT            MY_CTL_CODE(120)     // 恢复SSDT
#define CTL_RESTORE_SHADOW_SSDT     MY_CTL_CODE(121)     // 恢复ShadowSSDT
#define CTL_RESTORE_IDT             MY_CTL_CODE(122)     // 恢复IDT
```

#### 3.6.2 进程信息结构
```cpp
typedef struct PROCESS_INFO {
    ULONG ProcessId;                    // 进程ID
    ULONG ParentProcessId;              // 父进程ID
    CHAR ImageFileName[16];             // 进程名称（短名）
    PVOID EprocessAddr;                 // EPROCESS地址
    ULONG DirectoryTableBase;           // CR3页目录基地址
} *PPROCESS_INFO;
```

#### 3.6.3 模块信息结构
```cpp
typedef struct MODULE_INFO {
    CHAR Name[64];                      // 模块名称（短名）
    CHAR FullPath[256];                 // 模块完整路径
    PVOID ImageBase;                    // 模块基地址
    ULONG ImageSize;                    // 模块大小
    USHORT LoadOrderIndex;              // 加载顺序索引
    USHORT LoadCount;                   // 加载计数
} *PMODULE_INFO;
```

#### 3.6.4 系统调用信息结构
```cpp
typedef struct SSDT_INFO {
    ULONG Index;                        // 系统调用索引号
    PVOID FunctionAddress;              // 当前函数地址
    CHAR FunctionName[64];              // 函数名称
} *PSSDT_INFO, ShadowSSDT_INFO, *PShadowSSDT_INFO;

// 系统服务描述符表结构
typedef struct _SYSTEM_SERVICE_DESCRIPTOR_TABLE {
    PULONG Base;                        // 系统服务函数指针数组
    PULONG ServiceCounterTable;         // 服务调用计数表
    ULONG NumberOfServices;             // 服务数量
    PUCHAR ParamTableBase;              // 参数表
} SYSTEM_SERVICE_DESCRIPTOR_TABLE, *PSYSTEM_SERVICE_DESCRIPTOR_TABLE;
```

#### 3.6.5 回调信息结构
```cpp
// 回调类型枚举
typedef enum CALLBACK_TYPE {
    TypeProcess = 0,                    // PsSetCreateProcessNotifyRoutine - 进程创建/终止
    TypeProcessEx = 10,                 // PsSetCreateProcessNotifyRoutineEx - 进程扩展回调
    TypeThread = 1,                     // PsSetCreateThreadNotifyRoutine - 线程创建/终止
    TypeImage = 2,                      // PsSetLoadImageNotifyRoutine - 模块/映像加载
    TypeRegistry = 3,                   // CmRegisterCallback - 注册表操作
    TypeObject = 4,                     // ObRegisterCallbacks - 对象操作
    TypeBugCheck = 5,                   // KeRegisterBugCheckCallback - 系统崩溃
    TypeShutdown = 6,                   // IoRegisterShutdownNotification - 系统关闭
    TypeBugCheckReason = 7,             // KeRegisterBugCheckReasonCallback - 蓝屏原因
    TypeAll                             // 枚举所有回调
} *PCALLBACK_TYPE;

typedef struct CALLBACK_INFO {
    CALLBACK_TYPE Type;                 // 回调类型
    ULONG Index;                        // 在回调数组中的索引
    PVOID CallbackEntry;                // 回调入口地址
    CHAR ModulePath[256];               // 模块完整路径
    BOOLEAN IsValid;                    // 回调是否有效
    
    union {
        PVOID CallbackExtra;            // 通用额外信息
        UCHAR Reserved[32];             // 保留字段
        struct {
            CHAR ObjectTypeName[32];    // 对象类型名
            PVOID ObjTypeAddr;          // 对象类型地址
            PVOID CallbackRegistration; // ObUnRegisterCallbacks需要的参数
        } ObjectExtra;
    } Extra;
} *PCALLBACK_INFO;
```

#### 3.6.6 中断描述符结构
```cpp
// 中断描述符表寄存器结构
#pragma pack(push, 1)
typedef struct IDTR {
    unsigned short Limit;               // IDT限制
    ULONG_PTR Base;                     // IDT基址
} *PIDTR;
#pragma pack(pop)

// 中断描述符结构 (x64)
typedef struct InterruptDescriptor {
    USHORT OffsetLow;                   // 处理程序地址低16位
    USHORT Selector;                    // 段选择子
    USHORT IstIndex : 3;                // IST索引
    USHORT Reserved0 : 5;               // 保留
    USHORT Type : 4;                    // 门类型
    USHORT Reserved1 : 1;               // 保留
    USHORT Dpl : 2;                     // 描述符特权级
    USHORT Present : 1;                 // 存在位
    USHORT OffsetMiddle;                // 处理程序地址中16位
    ULONG OffsetHigh;                   // 处理程序地址高32位
    ULONG Reserved2;                    // 保留
} *PINTDESC;

typedef struct _IDT_INFO {
    ULONG CpuId;                        // CPU索引
    ULONG id;                           // 数组下标
    CHAR funcName[128];                 // 函数名
    USHORT Selector;                    // 段选择子
    USHORT Dpl;                         // 描述符特权级
    ULONG_PTR Address;                  // 处理程序地址
    CHAR Path[256];                     // 模块路径
    ULONG Type;                         // 描述符类型
} IDT_INFO, *PIDT_INFO;
```

#### 3.6.7 其他重要结构

**GDT相关结构**
```cpp
#pragma pack(push, 1)
typedef struct GDTR {
    unsigned short Limit;               // GDT限制
    ULONG_PTR Base;                     // GDT基址
} *PGDTR;
#pragma pack(pop)

typedef struct SegmentDescriptor {
    unsigned Limit1 : 16;               // 界限低16位
    unsigned Base1 : 16;                // 基址低16位
    unsigned Base2 : 8;                 // 基址中8位
    unsigned type : 4;                  // 段类型
    unsigned s : 1;                     // 系统段标志
    unsigned dpl : 2;                   // 特权级
    unsigned p : 1;                     // 存在位
    unsigned Limit2 : 4;                // 界限高4位
    unsigned avl : 1;                   // 软件可用位
    unsigned l : 1;                     // 64位代码段标志
    unsigned db : 1;                    // 操作数大小
    unsigned g : 1;                     // 粒度位
    unsigned Base3 : 8;                 // 基址高8位
} *PSEGDESC;
```

**网络监控结构**
```cpp
typedef struct NETWORK_PORT_INFO {
    CHAR Protocol[16];                  // 协议 (TCP/UDP)
    CHAR LocalAddress[64];              // 本地地址 (IP:Port)
    CHAR RemoteAddress[64];             // 外部地址 (IP:Port)
    CHAR State[32];                     // 状态 (LISTENING, ESTABLISHED...)
    ULONG_PTR ConnectionId;             // 连接ID
    ULONG ProcessId;                    // 进程ID
    CHAR ProcessPath[256];              // 进程路径
} *PNETWORK_PORT_INFO;
```

**文件操作结构**
```cpp
typedef struct FILE_REQ {
    WCHAR FilePath[MAX_PATH];           // 文件路径（解锁/删除）
} *PFILE_REQ;
```

### 3.7 功能要点设计

#### 3.7.1 进程枚举算法 (`process.cpp`)

**1. ActiveProcessLinks链表遍历算法**

```cpp
// 实现函数：EnumProcessFromLinksEx()
// 数据流：PsInitialSystemProcess → ActiveProcessLinks遍历 → EPROCESS结构解析

算法流程：
1. 初始化PDB偏移：通过PDB符号获取EPROCESS结构各字段偏移
   - ProcessId: "_EPROCESS.UniqueProcessId"
   - ParentProcessId: "_EPROCESS.InheritedFromUniqueProcessId"  
   - ImageFileName: "_EPROCESS.ImageFileName"
   - ActiveProcessLinks: "_EPROCESS.ActiveProcessLinks"
   - DirectoryTableBase: "_EPROCESS.Pcb.DirectoryTableBase"

2. 链表遍历逻辑：
   CurrentProcess = PsInitialSystemProcess  // 从系统进程开始
   do {
       提取进程信息到PROCESS_INFO结构
       pListEntry = CurrentProcess + ActiveProcessLinks偏移
       CurrentProcess = CONTAINING_RECORD(pListEntry->Flink, EPROCESS, ActiveProcessLinks)
   } while (CurrentProcess != StartProcess)

3. 数据提取：每个进程提取PID、PPID、进程名、EPROCESS地址、页目录基址
```

**2. 内存搜索EPROCESS算法**
```cpp
// 实现函数：EnumProcessBySearchMem()  

算法流程：
1. 搜索范围确定：
   PEPROCESS systemProcess = PsLookupProcessByProcessId(4)  // 系统进程
   startAddr = systemProcess地址 & ~0xFFFFFFF  // 256MB对齐
   endAddr = startAddr + 0x10000000  // 256MB搜索范围

2. 16字节对齐扫描：
   for (addr = startAddr; addr < endAddr; addr += 0x10) {
       if (ValidateEPROCESS(addr)) {
           提取进程信息
       }
   }

3. EPROCESS特征：
Dispatcher Type == 0x03（进程对象标识）
PID范围合理性（4-65536）
父进程ID有效性（0-65536）
进程名长度检查（≤16字符）
ActiveProcessLinks链表完整性（Flink->Blink == 当前节点）
DirectoryTableBase页对齐（低12位为0且非0）
```

#### 3.7.2 模块枚举算法 (`module.cpp`)

**.隐藏驱动检测算法**

```cpp
// 数据流：标准枚举 + PE头内存搜索 → 交叉比对 → 隐藏驱动识别

算法流程：
1. 建立标准驱动集合：
   通过EnumModuleEx()获取所有已知驱动基址
   存储到std::set<ULONG_PTR>用于快速查找

2. 内核地址空间PE头搜索：
   for (addr = 0xFFFFF80000000000; addr < 0xFFFFFFFFFFFFF000; addr += 0x1000) {
       验证DOS头：addr->e_magic == IMAGE_DOS_SIGNATURE
       验证PE头：(addr + e_lfanew)->Signature == IMAGE_NT_SIGNATURE  
       验证驱动特征：FileHeader.Characteristics & IMAGE_FILE_DLL
   }

3. 交叉比对检测：
   if (knownDrivers.find(pe_base_addr) == knownDrivers.end()) {
       发现隐藏驱动 → 提取导出表名称 → 记录到结果集
   }
```

#### 3.7.3 系统调用表分析算法 (`ssdt.cpp`)

**1. SSDT枚举算法**
```cpp
// 数据流：KeServiceDescriptorTable → 地址解码 → PDB符号解析

算法流程：
1. 获取SSDT基址：
   PSYSTEM_SERVICE_DESCRIPTOR_TABLE pSSDT = KeServiceDescriptorTable
   numberOfServices = pSSDT->NumberOfServices

2. 函数地址解码：
   for (index = 0; index < numberOfServices; index++) {
       encodedAddr = pSSDT->Base[index]
       realAddr = SSDT_GetPfnAddr(index, pSSDT->Base)  // 解码算法
   }

3. 符号解析：
   通过PDB符号将函数地址解析为函数名
   检测Hook：对比当前地址vs PDB原始地址
```

**2. ShadowSSDT分析算法**

```cpp
// 数据流：KeServiceDescriptorTableShadow[1] → Win32k函数解析

算法流程：
1. 获取ShadowSSDT基址：
   PSYSTEM_SERVICE_DESCRIPTOR_TABLE ShadowTableArray = KeServiceDescriptorTableShadow
   PSYSTEM_SERVICE_DESCRIPTOR_TABLE pShadowSSDT = &ShadowTableArray[1]

2. Win32k函数枚举：
   使用预定义的WIN7_SHADOW_SSDT_FUNCTIONS数组
   或通过win32k.sys PDB符号解析获取函数名

3. 地址解码：
   与SSDT相同，使用SSDT_GetPfnAddr解码压缩地址格式
```

#### 3.7.4 系统回调枚举算法 (`callback.cpp`)

**1. 进程/线程/映像回调算法**
```cpp
// 数据流：全局回调数组 → 回调函数地址 → 模块路径解析

算法流程：
1. 获取回调数组基址：
   - PsSetCreateProcessNotifyRoutine: "PspCreateProcessNotifyRoutine" 
   - PsSetCreateThreadNotifyRoutine: "PspCreateThreadNotifyRoutine"
   - PsSetLoadImageNotifyRoutine: "PspLoadImageNotifyRoutine"

2. 数组遍历：
   for (index = 0; index < MAX_CALLBACK_COUNT; index++) {
       callbackPtr = callbackArray[index]
       if (callbackPtr && 验证回调有效性) {
           通过FindModuleByAddress解析模块路径
       }
   }
```

**2. 对象回调枚举算法**
```cpp
// 实现函数：EnumCallbacks(TypeObject)
// 数据流：ObTypeIndexTable → _OBJECT_TYPE → CallbackList链表

算法流程：
1. 遍历对象类型表：
   ULONG_PTR ObTypeIndexTable = ntos.GetPointer("ObTypeIndexTable")
   for (i = 0; i < 100; i++) {
       objTypeAddr = *(ULONG_PTR*)(ObTypeIndexTable + i * sizeof(ULONG_PTR))
   }

2. 获取对象类型信息：
   UNICODE_STRING* typeName = objTypeAddr + "Name"偏移
   LIST_ENTRY* callbackListHead = objTypeAddr + "CallbackList"偏移

3. 遍历回调链表：
   for (entry = head->Flink; entry != head; entry = entry->Flink) {
       CALLBACK_BODY* cb = CONTAINING_RECORD(entry, CALLBACK_BODY, ListEntry)
       处理PreCallbackRoutine和PostCallbackRoutine
       保存CallbackNode用于后续删除操作
   }
```

#### 3.7.5 PDB符号解析算法

**1. 符号文件加载算法**
```cpp
// 数据流：PDB文件路径 → 符号表加载 → 结构偏移/函数地址查询

算法流程：
1. PDB文件加载：
   加载ntoskrnl.exe.pdb（内核符号）
   加载win32k.sys.pdb（图形子系统符号）

2. 结构偏移查询：
   OFFSET(offset, "StructName", "FieldName")
   通过符号信息计算字段在结构中的偏移量

3. 函数地址查询：  
   GetPointer("FunctionName") → 获取函数虚拟地址
   GetPointerRVA("FunctionName") → 获取函数相对虚拟地址
```

**2. 地址编码/解码算法**
```cpp
// SSDT压缩地址格式处理

编码算法：SSDT_EncodePfnAddr()
offset = functionAddr - ssdtBase
if (offset < 0) {
    return ((ULONG)(-offset) << 4) | 0x80000000  // 负偏移+符号位
} else {
    return (ULONG)(offset << 4)  // 正偏移左移4位
}

解码算法：SSDT_GetPfnAddr()
encodedValue = ssdtBase[index]  
if (encodedValue & 0x80000000) {
    offset = -((LONG)(encodedValue << 1) >> 5)  // 处理负偏移
} else {
    offset = (LONG)(encodedValue >> 4)  // 处理正偏移
}
return ssdtBase + offset
```

### 类设计

#### 
