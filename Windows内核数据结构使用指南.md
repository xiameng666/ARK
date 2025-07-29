# Windows内核数据结构使用指南

## 概述
本文档总结了Windows内核开发中常用的数据结构及其使用方法，特别针对系统监控、进程分析、驱动开发等ARK工具场景。

## 核心内核数据结构

### 1. EPROCESS (Executive Process) - 进程对象
**用途**: 内核中最重要的进程管理结构，包含进程的所有关键信息

#### 关键字段和偏移
```cpp
// process.cpp:11-14 中使用的偏移获取
OFFSET(procMeta.ProcessId, "_EPROCESS", "UniqueProcessId");          // 进程ID
OFFSET(procMeta.ActiveProcessLinks, "_EPROCESS", "ActiveProcessLinks"); // 进程链表
OFFSET(procMeta.ParentProcessId, "_EPROCESS", "InheritedFromUniqueProcessId"); // 父进程ID
OFFSET(procMeta.ImageFileName, "_EPROCESS", "ImageFileName");        // 进程名称
OFFSET(pcbOffset, "_EPROCESS", "Pcb");                              // 进程控制块
```

#### 使用示例
```cpp
// 遍历所有进程 (process.cpp:33-65)
PEPROCESS CurrentProcess = PsGetCurrentProcess();
PEPROCESS StartProcess = CurrentProcess;

do {
    // 获取进程信息
    ULONG pid = *(PULONG)((PUCHAR)CurrentProcess + procMeta.ProcessId);
    PCHAR name = (PCHAR)((PUCHAR)CurrentProcess + procMeta.ImageFileName);
    
    // 移动到下一个进程
    PLIST_ENTRY pNextListEntry = ((PLIST_ENTRY)((PUCHAR)CurrentProcess + procMeta.ActiveProcessLinks))->Flink;
    CurrentProcess = (PEPROCESS)((PUCHAR)pNextListEntry - procMeta.ActiveProcessLinks);
    
} while (CurrentProcess != StartProcess);
```

#### 为什么这样做？
- **链表遍历**: EPROCESS通过ActiveProcessLinks形成双向链表，这是系统内部维护所有进程的方式
- **偏移访问**: 由于不同Windows版本结构体布局可能不同，使用动态偏移确保兼容性
- **数据流**: 系统调度器 → EPROCESS链表 → 我们的枚举代码 → 用户界面显示

### 2. ETHREAD (Executive Thread) - 线程对象
**用途**: 管理系统中的线程信息

#### 关键字段
```cpp
// process.cpp:10 中的关联
OFFSET(procMeta.EThreadToProcess, "_ETHREAD", "ThreadsProcess"); // 指向所属EPROCESS
```

#### 数据流分析
线程创建时: `PsCreateSystemThreadEx` → 分配`ETHREAD` → 链接到所属`EPROCESS` → 加入调度队列

### 3. DRIVER_OBJECT - 驱动对象
**用途**: 每个内核驱动都有对应的DRIVER_OBJECT，包含驱动的所有管理信息

#### 结构分析 (DriverBase.cpp:323, 341)
```cpp
typedef struct _DRIVER_OBJECT {
    CSHORT Type;                    // 对象类型
    CSHORT Size;                    // 对象大小
    PDEVICE_OBJECT DeviceObject;    // 设备对象链表
    ULONG Flags;                    // 驱动标志
    PVOID DriverStart;              // 驱动映像起始地址
    ULONG DriverSize;               // 驱动大小
    PVOID DriverSection;            // 驱动段信息
    PDRIVER_EXTENSION DriverExtension; // 驱动扩展
    UNICODE_STRING DriverName;      // 驱动名称
    PUNICODE_STRING HardwareDatabase; // 硬件数据库路径
    PFAST_IO_DISPATCH FastIoDispatch;  // 快速I/O调度表
    PDRIVER_INITIALIZE DriverInit;     // 驱动初始化例程
    PDRIVER_STARTIO DriverStartIo;     // 启动I/O例程
    PDRIVER_UNLOAD DriverUnload;       // 卸载例程
    PDRIVER_DISPATCH MajorFunction[IRP_MJ_MAXIMUM_FUNCTION + 1]; // IRP主功能调度表
} DRIVER_OBJECT, *PDRIVER_OBJECT;
```

#### Hook检测应用 (driver.cpp:27-79)
```cpp
// 检测IRP调度函数是否被Hook
PDEVICE_OBJECT CurrentDev = DriverObj->DeviceObject;
while (CurrentDev) {
    // 检查设备栈
    PDEVICE_OBJECT AttachedDev = CurrentDev->AttachedDevice;
    if (AttachedDev) {
        PDRIVER_OBJECT AttachedDriver = AttachedDev->DriverObject;
        // 分析是否存在恶意过滤驱动
    }
    CurrentDev = CurrentDev->NextDevice;
}
```

#### 为什么检测驱动Hook？
- **安全考虑**: 恶意软件常通过Hook驱动派遣函数来拦截系统调用
- **数据流**: 应用程序 → IRP → 驱动MajorFunction → 实际处理函数
- **检测原理**: 正常情况下，MajorFunction指向驱动模块内部，如果指向其他模块则可能被Hook

### 4. DEVICE_OBJECT - 设备对象
**用途**: 代表系统中的一个设备，与DRIVER_OBJECT密切相关

#### 设备栈分析 (driver.cpp:75-85)
```cpp
typedef struct _DEVICE_OBJECT {
    CSHORT Type;                    // 对象类型
    USHORT Size;                    // 对象大小
    LONG ReferenceCount;            // 引用计数
    PDRIVER_OBJECT DriverObject;    // 指向拥有此设备的驱动
    PDEVICE_OBJECT NextDevice;      // 同一驱动的下一个设备
    PDEVICE_OBJECT AttachedDevice;  // 附加的设备(过滤驱动)
    PIRP CurrentIrp;                // 当前正在处理的IRP
    // ... 更多字段
} DEVICE_OBJECT, *PDEVICE_OBJECT;
```

#### 实际应用场景
设备栈检测恶意过滤驱动：
```cpp
// 遍历设备栈，检测过滤驱动
PDEVICE_OBJECT AttachedDev = CurrentDev->AttachedDevice;
while (AttachedDev) {
    // 记录过滤驱动信息
    FilterInfo.DriverObject = (ULONG_PTR)AttachedDev->DriverObject;
    FilterInfo.DeviceObject = (ULONG_PTR)AttachedDev;
    // 获取驱动名称用于分析
    AttachedDev = AttachedDev->AttachedDevice;
}
```

### 5. 自定义通信结构

#### PROCESS_INFO - 进程信息结构
```cpp
typedef struct PROCESS_INFO {
    ULONG ProcessId;                    // 进程ID
    ULONG ParentProcessId;              // 父进程ID  
    CHAR ImageFileName[16];             // 进程名称
    PVOID EprocessAddr;                 // EPROCESS地址(用于进一步分析)
    ULONG DirectoryTableBase;           // CR3页目录基地址(内存分析)
} *PPROCESS_INFO;
```

#### MODULE_INFO - 模块信息结构
```cpp
typedef struct MODULE_INFO {
    CHAR Name[64];                      // 模块名称
    CHAR FullPath[256];                 // 完整路径
    PVOID ImageBase;                    // 基地址
    ULONG ImageSize;                    // 大小
    USHORT LoadOrderIndex;              // 加载顺序
    USHORT LoadCount;                   // 引用计数
} *PMODULE_INFO;
```

## 回调机制数据结构

### EX_CALLBACK 系列 (callback.h:18-32)
**用途**: Windows回调机制的核心结构

```cpp
typedef struct _EX_CALLBACK {
    EX_FAST_REF RoutineBlock;          // 快速引用到回调例程块
} EX_CALLBACK, *PEX_CALLBACK;

typedef struct _EX_CALLBACK_ROUTINE_BLOCK {
    EX_RUNDOWN_REF RundownProtect;     // 运行保护
    PEX_CALLBACK_FUNCTION Function;    // 回调函数指针
    PVOID Context;                     // 上下文参数
} EX_CALLBACK_ROUTINE_BLOCK, *PEX_CALLBACK_ROUTINE_BLOCK;
```

#### 回调检测原理
1. **数据流**: 系统事件 → 回调分发器 → 注册的回调函数 → 用户处理代码
2. **检测方法**: 遍历回调数组，分析Function指针是否指向可疑模块
3. **安全意义**: 恶意软件经常注册回调来监控系统活动

### 蓝屏回调结构 (callback.h:43-52)
```cpp
typedef struct _KBUGCHECK_CALLBACK_RECORD {
    LIST_ENTRY Entry;                   // 链表入口
    PKBUGCHECK_CALLBACK_ROUTINE CallbackRoutine; // 回调函数
    PVOID Buffer;                       // 缓冲区
    ULONG Length;                       // 长度
    PUCHAR Component;                   // 组件名称
} KBUGCHECK_CALLBACK_RECORD, *PKBUGCHECK_CALLBACK_RECORD;
```

## SSDT相关结构

### SYSTEM_SERVICE_DESCRIPTOR_TABLE (proto.h:190-195)
**用途**: 系统服务描述符表，所有Native API的调度核心

```cpp
typedef struct _SYSTEM_SERVICE_DESCRIPTOR_TABLE {
    PULONG Base;                  // 系统服务函数指针数组
    PULONG ServiceCounterTable;   // 服务调用计数表  
    ULONG NumberOfServices;       // 服务数量
    PUCHAR ParamTableBase;        // 参数表
} SYSTEM_SERVICE_DESCRIPTOR_TABLE, *PSYSTEM_SERVICE_DESCRIPTOR_TABLE;
```

#### Hook检测应用
```cpp
// 检测SSDT Hook
PVOID CurrentAddr = (PVOID)((PULONG)KeServiceDescriptorTable->Base)[Index];
PVOID OriginalAddr = GetOriginalSSDTAddress(Index); // 从原始内核获取
if (CurrentAddr != OriginalAddr) {
    // 检测到Hook
    Log("SSDT[%d] Hooked: %p -> %p", Index, OriginalAddr, CurrentAddr);
}
```

## 数据结构使用最佳实践

### 1. 偏移动态获取
```cpp
// 不要硬编码偏移，使用运行时获取
ULONG offset = GetFieldOffset("_EPROCESS", "ActiveProcessLinks");
PLIST_ENTRY links = (PLIST_ENTRY)((PUCHAR)process + offset);
```

### 2. 异常处理
```cpp
__try {
    // 访问内核结构
    ULONG pid = *(PULONG)((PUCHAR)process + pidOffset);
} __except(EXCEPTION_EXECUTE_HANDLER) {
    // 处理访问违例
    Log("Failed to access process structure");
}  
```

### 3. 内存管理
```cpp
// 分配带标签的内存
PVOID buffer = ExAllocatePoolWithTag(NonPagedPool, size, 'kra ');
if (buffer) {
    // 使用buffer
    ExFreePoolWithTag(buffer, 'kra ');
}
```

### 4. 同步机制
```cpp
// 使用自旋锁保护数据结构
KIRQL oldIrql;
KeAcquireSpinLock(&g_SpinLock, &oldIrql);
// 访问共享数据结构
KeReleaseSpinLock(&g_SpinLock, oldIrql);
```

## 思维链路总结

### 数据流动路径
1. **用户态请求** → DeviceIoControl → **内核态处理**
2. **内核枚举** → 遍历内核结构 → **提取信息** → 返回用户态
3. **实时监控** → 注册回调 → **事件触发** → 记录到缓冲区 → 用户态读取

### 为什么使用这些结构？
- **EPROCESS**: 系统进程管理的核心，包含最完整的进程信息
- **DRIVER_OBJECT**: 驱动管理和Hook检测的基础
- **回调结构**: 实时监控系统事件的最有效方式
- **SSDT**: 系统调用拦截和分析的关键入口点

### 学习要点
1. **理解数据关联**: EPROCESS ↔ ETHREAD ↔ MODULE，这些结构相互关联
2. **掌握遍历方法**: 链表遍历是内核编程的基本技能
3. **重视版本兼容**: 不同Windows版本结构布局可能不同
4. **注意安全检查**: 访问内核结构前必须验证指针有效性
5. **理解Hook原理**: 大多数安全检测都基于对这些结构的分析

## 调试技巧
- 使用WinDbg的`dt`命令查看结构体布局: `dt nt!_EPROCESS`
- 使用`!process 0 0`命令列出所有进程的EPROCESS地址
- 利用`!object`命令分析对象管理器中的对象

这份指南涵盖了ARK项目中使用的主要内核数据结构，理解这些结构的用途和相互关系是深入Windows内核开发的关键。