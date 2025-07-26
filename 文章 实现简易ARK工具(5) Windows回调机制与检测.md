# 前言

​	大家好，本篇文章讨论Windows内核回调机制在ARK工具中的实现与检测。回调机制是Windows内核提供的一种强大的监控手段，也是恶意软件经常滥用的技术。作为ARK工具开发者，我们需要深入理解这套机制，既要会用它来实现监控功能，也要能检测和清除恶意回调。

​	感觉这个系列越写越深入了，从GDT、SSDT到现在的回调机制，每一个都是Windows内核的精华所在。受众依然是和我一样在学习Windows内核的朋友们，希望能帮到大家。

​	作者依然是在学习中的小白，如有错误恳请指正，谢谢你的时间！

**随便bb**

​	写这个回调检测的时候发现，微软设计这套机制真的很巧妙。既保证了内核的稳定性，又提供了灵活的监控能力。但同时也给了恶意软件很多可乘之机，这就是矛与盾的关系吧。现在很多EDR和杀毒软件都大量使用回调机制，而恶意软件也在想方设法绕过或者利用这些回调。

​	另外，研究这些机制的时候建议大家多看WRK（Windows Research Kernel）的源码，虽然是老版本，但核心思想都是一致的。

# 效果图

我们实现的回调检测功能：

![image-20250101000001](.\images\callback_enum.png)

![image-20250101000002](.\images\callback_delete.png)

# 需要知道的概念

## 什么是回调机制

Windows内核回调机制是一种**事件驱动**的通知系统。当系统中发生特定事件时（如进程创建、模块加载等），内核会自动调用之前注册的回调函数。这种机制让第三方代码能够监控系统行为，实现安全防护、行为分析等功能。

**人话解释**：就像你在QQ群里设置了关键词提醒，一旦有人说了这个词，你就会收到通知。回调机制就是在内核里设置这样的"关键事件提醒"，一旦发生指定事件，你的函数就会被调用。

Windows内核支持多种类型的回调：

- **进程创建回调**：PsSetCreateProcessNotifyRoutine
- **线程创建回调**：PsSetCreateThreadNotifyRoutine  
- **模块加载回调**：PsSetLoadImageNotifyRoutine
- **注册表回调**：CmRegisterCallback
- **对象回调**：ObRegisterCallbacks
- **电源管理回调**：PoRegisterPowerSettingCallback

## 回调的底层结构

要理解回调机制，必须先了解内核是如何组织这些回调函数的。以进程创建回调为例：

```cpp
// 快速引用结构 - 用于原子操作
typedef struct _EX_FAST_REF {
    union {
        PVOID Object;          // 指向EX_CALLBACK_ROUTINE_BLOCK的指针
        ULONG_PTR RefCnt : 4;  // 引用计数 (低4位)
        ULONG_PTR Value;       // 完整的值
    };
} EX_FAST_REF, *PEX_FAST_REF;

// 回调数组中的元素
typedef struct _EX_CALLBACK {
    EX_FAST_REF RoutineBlock;   // 指向实际的回调数据块
} EX_CALLBACK, *PEX_CALLBACK;

// 实际的回调函数块
typedef struct _EX_CALLBACK_ROUTINE_BLOCK {
    EX_RUNDOWN_REF        RundownProtect;   // 运行时保护
    PEX_CALLBACK_FUNCTION Function;         // 回调函数地址 ←← 这就是我们要找的！
    PVOID                 Context;          // 回调上下文
} EX_CALLBACK_ROUTINE_BLOCK, *PEX_CALLBACK_ROUTINE_BLOCK;
```

内核中存在几个全局数组来管理这些回调：

```cpp
// 这些符号在内核中定义，我们需要通过PDB解析获取地址
EX_CALLBACK PspCreateProcessNotifyRoutine[64];  // 进程创建回调数组
EX_CALLBACK PspCreateThreadNotifyRoutine[64];   // 线程创建回调数组  
EX_CALLBACK PspLoadImageNotifyRoutine[64];      // 镜像加载回调数组
```

**关键理解**：
1. Windows用**数组**来存储回调函数指针
2. 每个数组元素指向一个**EX_CALLBACK_ROUTINE_BLOCK**结构
3. 真正的回调函数地址存储在**Function**字段中
4. 使用**EX_FAST_REF**是为了支持无锁的原子操作

## 为什么需要检测回调

回调机制本身是正当的，但恶意软件经常滥用它：

1. **Rootkit隐藏**：通过进程创建回调隐藏恶意进程
2. **反调试**：通过线程创建回调检测调试器
3. **文件保护**：通过镜像加载回调阻止特定程序运行
4. **数据窃取**：通过注册表回调监控敏感信息访问
5. **持久化**：利用回调机制实现无文件驻留

作为ARK工具，我们需要：
- **枚举所有回调**：找出系统中注册的回调函数
- **分析回调来源**：判断是否来自可信模块
- **清除恶意回调**：移除可疑的回调函数

# 实现原理

## 获取回调数组地址

要枚举回调，首先需要找到内核中存储回调的数组。这些数组的符号没有导出，需要通过PDB文件解析：

```cpp
#define INIT_PDB \
    NTSTATUS status; \
    oxygenPdb ntos; \
    status = ntos.Download(); \
    if (!NT_SUCCESS(status)) { \
        Log("[XM] PDB下载失败: %08X", status); \
        return status; \
    }

// 通过PDB获取符号地址的通用宏
#define GET_CALLBACK_FUNCTION_INFO(symbolName, WhatType)\
    INIT_PDB;\
    ULONG_PTR processRoutineEntry = ntos.GetPointer(symbolName);\
    Log("[XM] %s地址: %p", symbolName, processRoutineEntry);\
    PEX_CALLBACK callbackArray = (PEX_CALLBACK)processRoutineEntry;\
    for (ULONG i = 0; i < 64; i++)\
    {\
        EX_FAST_REF* fastRef = &callbackArray[i].RoutineBlock;\
        PVOID objectPtr = (PVOID)(fastRef->Value & ~0xF);\
        PEX_CALLBACK_ROUTINE_BLOCK pObj = (PEX_CALLBACK_ROUTINE_BLOCK)objectPtr;\
        if (pObj) {\
            PCALLBACK_INFO info = &callbackBuffer[count];\
            RtlZeroMemory(info, sizeof(CALLBACK_INFO));\
            info->Type = WhatType;\
            info->Index = i;\
            info->CallbackEntry = pObj->Function;\
            info->IsValid = TRUE;\
            count++;\
        }\
    }
```

**核心技巧解析**：

1. **PDB符号解析**：通过`ntos.GetPointer(symbolName)`获取内核符号地址
2. **指针运算**：`fastRef->Value & ~0xF` 清除低4位的引用计数，获取真正的对象指针
3. **数组遍历**：遍历64个数组元素（Windows的回调数组大小限制）
4. **有效性检查**：检查对象指针是否为空

## 枚举回调实现

```cpp
NTSTATUS EnumCallbacks(PCALLBACK_INFO callbackBuffer, CALLBACK_TYPE type, PULONG callbackCount) {
    if (!callbackBuffer || !callbackCount) {
        return STATUS_INVALID_PARAMETER;
    }
    
    *callbackCount = 0;
    ULONG count = 0;

    switch (type) {
        case TypeProcess:  // 进程创建回调
        {
            GET_CALLBACK_FUNCTION_INFO("PspCreateProcessNotifyRoutine", TypeProcess)
        }
        break;

        case TypeThread:   // 线程创建回调
        {
            GET_CALLBACK_FUNCTION_INFO("PspCreateThreadNotifyRoutine", TypeThread)
        }
        break;

        case TypeImage:    // 镜像加载回调
        {
            GET_CALLBACK_FUNCTION_INFO("PspLoadImageNotifyRoutine", TypeImage)
        }
        break;

        case TypeRegistry:
        case TypeObject:
        case TypeBugCheck:
        case TypeShutdown:
            // TODO: 实现其他类型的回调枚举
            break;

        default:
            return STATUS_NOT_SUPPORTED;
    }
        
    *callbackCount = count;
    Log("[XM] EnumCallbacks: 类型 %d, 返回 %d 个回调", type, count);
    return STATUS_SUCCESS;
}
```

## 删除回调实现

删除回调需要调用对应的Windows API：

```cpp
NTSTATUS DeleteCallback(CALLBACK_TYPE type, ULONG index, PVOID addr) {
    Log("[XM] DeleteCallback: 类型=%d，索引=%d 地址=%p", type, index, addr);
    
    switch (type) {
        case TypeProcess:
        {
            // 先获取原始函数地址
            INIT_PDB;
            ULONG_PTR arrayBase = ntos.GetPointer("PspCreateProcessNotifyRoutine");
            PEX_CALLBACK callbackArray = (PEX_CALLBACK)arrayBase;
            EX_FAST_REF* fastRef = &callbackArray[index].RoutineBlock;
            PVOID objectPtr = (PVOID)(fastRef->Value & ~0xF);
            PEX_CALLBACK_ROUTINE_BLOCK block = (PEX_CALLBACK_ROUTINE_BLOCK)objectPtr;
            PVOID originalFunction = block->Function;
            
            // 调用系统API删除回调
            NTSTATUS status = PsSetCreateProcessNotifyRoutine(
                (PCREATE_PROCESS_NOTIFY_ROUTINE)originalFunction,
                TRUE  // Remove = TRUE
            );
            return status;
        }
            
        case TypeThread:
        {
            // 类似的处理流程...
            NTSTATUS status = PsRemoveCreateThreadNotifyRoutine(
                (PCREATE_THREAD_NOTIFY_ROUTINE)originalFunction
            );
            return status;
        }
        
        case TypeImage:
        {
            // 类似的处理流程...
            NTSTATUS status = PsRemoveLoadImageNotifyRoutine(
                (PLOAD_IMAGE_NOTIFY_ROUTINE)originalFunction
            );
            return status;
        }
    }
    
    return STATUS_SUCCESS;
}
```

**重要细节**：

1. **获取原始函数**：删除回调时需要提供原始的函数地址
2. **使用正确的API**：不同类型的回调有不同的删除API
3. **Remove标志**：PsSetCreateProcessNotifyRoutine的第二个参数设为TRUE表示删除
4. **错误处理**：正确处理各种错误情况

# 安全考虑与防护

## 常见的回调攻击手法

**1. 回调劫持**
```cpp
// 恶意代码可能这样劫持回调
PVOID OriginalCallback = NULL;

VOID MaliciousCallback(HANDLE ProcessId, HANDLE ThreadId, BOOLEAN Create) {
    // 隐藏特定进程
    if (ProcessId == MaliciousProcessId) {
        return; // 不调用原始回调，实现隐藏
    }
    
    // 调用原始回调
    if (OriginalCallback) {
        ((PCREATE_PROCESS_NOTIFY_ROUTINE)OriginalCallback)(ProcessId, ThreadId, Create);
    }
}
```

**2. 回调链投毒**
恶意软件可能在回调链中插入恶意函数，干扰正常的系统监控。

**3. 反调试回调**
```cpp
VOID AntiDebugCallback(HANDLE ProcessId, HANDLE ThreadId, BOOLEAN Create) {
    if (IsDebuggerProcess(ProcessId)) {
        // 检测到调试器，执行反制措施
        KillProcess(ProcessId);
        CrashSystem();
    }
}
```

## ARK检测策略

**1. 白名单验证**
```cpp
BOOLEAN IsKnownGoodModule(PVOID CallbackAddress) {
    // 检查回调地址是否在已知的良性模块中
    PKLDR_DATA_TABLE_ENTRY module = GetModuleByAddress(CallbackAddress);
    if (module) {
        // 验证数字签名
        if (VerifyModuleSignature(module)) {
            return TRUE;
        }
    }
    return FALSE;
}
```

**2. 行为分析**
- 监控回调的注册时间和频率
- 分析回调函数的代码特征
- 检测异常的回调链修改

**3. 完整性检查**
```cpp
VOID CheckCallbackIntegrity() {
    // 定期检查回调数组是否被篡改
    static ULONG LastCallbackCount[TypeMax] = {0};
    
    for (int type = 0; type < TypeMax; type++) {
        ULONG currentCount = GetCallbackCount(type);
        if (currentCount != LastCallbackCount[type]) {
            Log("[ALERT] 回调数量发生变化: 类型=%d, 之前=%d, 现在=%d", 
                type, LastCallbackCount[type], currentCount);
            LastCallbackCount[type] = currentCount;
        }
    }
}
```

# 进阶话题

## 回调的执行上下文

了解回调的执行环境对于分析和防护很重要：

```cpp
VOID ProcessCallback(HANDLE ProcessId, HANDLE ThreadId, BOOLEAN Create) {
    // 当前执行环境信息
    KIRQL currentIrql = KeGetCurrentIrql();  // 通常是PASSIVE_LEVEL
    PEPROCESS currentProcess = PsGetCurrentProcess();  // 当前进程上下文
    PETHREAD currentThread = PsGetCurrentThread();     // 当前线程上下文
    
    Log("[CALLBACK] IRQL=%d, Process=%p, Thread=%p", 
        currentIrql, currentProcess, currentThread);
}
```

**关键点**：
- 回调通常在**PASSIVE_LEVEL**执行，可以访问分页内存
- 回调在**任意进程上下文**中执行，需要注意安全性
- 回调应该**快速返回**，避免影响系统性能

## 回调的同步机制

Windows使用多种同步机制保护回调数组：

```cpp
// EX_RUNDOWN_REF - 运行时保护机制
typedef struct _EX_RUNDOWN_REF {
    union {
        ULONG_PTR Count;     // 引用计数
        PVOID Ptr;           // 指针值
    };
} EX_RUNDOWN_REF, *PEX_RUNDOWN_REF;
```

这个机制确保：
1. **原子操作**：回调的添加/删除是原子的
2. **运行时保护**：正在执行的回调不会被删除
3. **引用计数**：跟踪回调的使用情况

## Hook检测进阶

**检测Inline Hook**：
```cpp
BOOLEAN CheckCallbackHook(PVOID CallbackAddress) {
    UCHAR originalBytes[16];
    UCHAR currentBytes[16];
    
    // 读取当前字节码
    if (!NT_SUCCESS(MmCopyVirtualMemory(
        PsGetCurrentProcess(),
        CallbackAddress,
        PsGetCurrentProcess(), 
        currentBytes,
        16,
        KernelMode,
        NULL))) {
        return FALSE;
    }
    
    // 检查是否有跳转指令 (0xE9, 0xEB, 0xEA等)
    if (currentBytes[0] == 0xE9 || currentBytes[0] == 0xEB) {
        Log("[ALERT] 检测到可能的Hook: %p", CallbackAddress);
        return TRUE;
    }
    
    return FALSE;
}
```

# 实验与测试

## 测试环境搭建

**推荐环境**：
- Windows 7 x64 (方便调试，符号完整)
- WinDbg + VMware (内核调试环境)  
- IDA Pro (静态分析)
- 测试用的恶意样本

**调试技巧**：
```
kd> dt nt!_EX_CALLBACK
   +0x000 RoutineBlock     : _EX_FAST_REF

kd> dt nt!_EX_CALLBACK_ROUTINE_BLOCK  
   +0x000 RundownProtect   : _EX_RUNDOWN_REF
   +0x008 Function         : Ptr64     void 
   +0x010 Context          : Ptr64 Void

kd> dd nt!PspCreateProcessNotifyRoutine L20
// 查看回调数组内容
```

## 实际应用案例

**案例1：检测进程隐藏Rootkit**
某些Rootkit通过注册进程创建回调，在进程枚举时过滤掉自己。通过我们的回调检测功能，可以发现这些异常的回调函数。

**案例2：反制恶意反调试**  
一些恶意软件注册线程创建回调来检测调试器。我们可以通过删除这些回调来绕过反调试。

**案例3：分析APT攻击**
高级持续性威胁常利用回调机制实现无文件驻留，通过分析回调可以发现攻击踪迹。

# 总结

Windows回调机制是内核安全的双刃剑。掌握回调的检测和管理技术，对于开发ARK工具和分析恶意软件都至关重要。

**核心要点**：
1. **理解数据结构**：EX_CALLBACK、EX_FAST_REF、EX_CALLBACK_ROUTINE_BLOCK
2. **掌握枚举技术**：通过PDB符号解析访问内核回调数组  
3. **学会清除回调**：使用合适的Windows API删除恶意回调
4. **建立检测策略**：白名单、行为分析、完整性检查
5. **注意安全细节**：执行上下文、同步机制、Hook检测

回调机制的学习永无止境，随着Windows系统的更新，新的回调类型和保护机制还在不断增加。建议大家多实践，多调试，在实战中加深理解。

**参考资料**：
- Windows Internals 7th Edition
- Windows Kernel Programming
- Rootkit Arsenal
- [看雪安全社区相关文章](https://bbs.kanxue.com/)

下一篇文章我们将讨论**对象回调（Object Callbacks）**的实现，这是Windows Vista之后引入的更强大的监控机制，敬请期待！

---

*如有技术问题或发现错误，欢迎在评论区指正交流！* 