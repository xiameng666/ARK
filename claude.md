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
- 主要数据结构: PROCESS_INFO, MODULE_INFO, CALLBACK_INFO, SSDT_INFO, IDT_INFO

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
| **IDT表** |
| IDT表项数量 | `CTL_ENUM_IDT_COUNT` | - | - | ✗ |
| IDT表数据 | `CTL_ENUM_IDT` | - | `ArkR3::IdtGetVec()` | ✓ |
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
| SSDT枚举 | `CTL_ENUM_SSDT` | `DriverBase.cpp:209` | `ArkR3.cpp:730` | ✓ |
| ShadowSSDT枚举 | `CTL_ENUM_ShadowSSDT` | `ssdt.cpp:55` | `ArkR3.cpp:785` | ✓ |
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



# TodoList

1. 进程检测隐藏
2. 驱动检测隐藏
3. 中断表恢复  
4. SSDT恢复  
5. ShadowSSDT恢复  



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
2) 恢复() - 完成

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

**8.** **IDT**表

1) 遍历 （已完成）- `CTL_ENUM_IDT`

**9.** **读写进程内存**

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
| Win32k枚举 | 枚举Win32k调用表 | `CTL_ENUM_ShadowSSDT` | `EnumShadowSSDT()`<br>`ssdt.cpp:55` | `ArkR3::ShadowSSDTGetVec()`<br>`ArkR3.cpp:785` | `ShadowSSDT_INFO` | ✓ |
| Win32k恢复 | 恢复Win32k HOOK | **待新增** | **待实现** | **待实现** | **待设计** | ✗ |
| **6. 中断表IDT** |
| IDT枚举 | 枚举中断描述符表 | **待新增** | **待实现** | **待实现** | **待设计** | ✗ |
| IDT恢复 | 恢复被HOOK的中断 | **待新增** | **待实现** | **待实现** | **待设计** | ✗ |
| **7. GDT全局描述符** |
| GDT枚举 | 枚举全局描述符表 | `CTL_GET_GDT_DATA` | `GetGDTData()`<br>`DriverBase.cpp:110` | `ArkR3::GetGDTData()` | `GDT_DATA_REQ`<br>`SegmentDescriptor` | ✓ |
| **8. IDT中断描述符** |
| IDT枚举 | 枚举中断描述符表 | `CTL_ENUM_IDT` | **待实现** | `ArkR3::IdtGetVec()` | `IDT_INFO` | ✓ |
| **9. 内存操作** |
| 直接内存读 | 读取内核/进程内存 | `CTL_READ_MEM` | **待实现** | **待实现** | `KERNEL_RW_REQ` | ✗ |
| 直接内存写 | 写入内核/进程内存 | `CTL_WRITE_MEM` | **待实现** | **待实现** | `KERNEL_RW_REQ` | ✗ |
| 附加进程读 | 附加到进程读内存 | `CTL_ATTACH_MEM_READ` | **待实现** | **待实现** | `PROCESS_MEM_REQ` | ✗ |
| 附加进程写 | 附加到进程写内存 | `CTL_ATTACH_MEM_WRITE` | **待实现** | **待实现** | `PROCESS_MEM_REQ` | ✗ |
| **10. 驱动分析** |
| 驱动枚举 | 枚举系统驱动对象 | `CTL_ENUM_DRIVER_COUNT`<br>`CTL_ENUM_DRIVER` | **待实现** | **待实现** | **待设计** | ✗ |
| 派遣函数Hook | 检测IRP派遣Hook | `CTL_ENUM_DISPATCH_HOOK` | `EnumDrvMJHooked()`<br>`driver.cpp:246` | **待实现** | `DISPATCH_HOOK_INFO` | ✓ |
| 设备栈分析 | 分析过滤驱动栈 | `CTL_ENUM_DEVICE_STACK` | `EnumDeviceStackAttach()`<br>`driver.cpp:9` | **待实现** | `DEVICE_STACK_INFO` | ✓ |
| **11. 网络监控** |
| 网络端口 | 枚举网络连接 | `CTL_ENUM_NETWORK_PORT` | `EnumNetworkPort()`<br>`network.cpp:4` | **R3实现** | `NETWORK_PORT_INFO` | ✓ |
| **12. 文件操作** |
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
8. **ShadowSSDT** - Win32k系统调用表分析 (**已完成**)
9. **IDT中断表** - 中断描述符表分析 (新增`idt.cpp`)

# ShadowSSDT 实现技术说明

## 完成状态：✅ 已实现

### 核心技术难点解决

**1. KeServiceDescriptorTableShadow 结构理解**
```cpp
// KeServiceDescriptorTableShadow 是包含2个元素的数组
SYSTEM_SERVICE_DESCRIPTOR_TABLE KeServiceDescriptorTableShadow[2] = {
    [0] - 普通SSDT (与KeServiceDescriptorTable相同)
    [1] - ShadowSSDT (Win32k系统调用，NtUser*/NtGdi*函数)
}
```

**2. 地址编码方式**
- **关键发现**: ShadowSSDT 使用与普通 SSDT **相同的压缩编码**
- **错误认知**: 之前以为ShadowSSDT存储直接RVA，实际需要 `SSDT_GetPfnAddr` 解码
- **数据验证**: 
  - 原始值: `0xFF962820` 
  - 解码后: `0x2282` (真实RVA)
  - 绝对地址: `win32k_base + 0x2282 = 0xFFFFC0CC3CBC2282` ✓

**3. 符号解析架构**
- **双PDB支持**: ntoskrnl.exe + win32k.sys 
- **智能路径解析**: win32k函数名通过独立PDB解析
- **地址计算**: 绝对地址 → RVA → 符号名

### 数据流动链路

```
R0驱动层:
KeServiceDescriptorTableShadow[1] → SSDT_GetPfnAddr解码 → 绝对地址

R0→R3通信:
DeviceIoControl(CTL_ENUM_ShadowSSDT) → SSDT_INFO数组传输

R3用户层:
绝对地址 → RVA计算 → win32k_pdb符号解析 → NtUser*/NtGdi*函数名
```

### 实现文件

| 层级 | 文件 | 核心函数 | 功能 |
|------|------|----------|------|
| R0 | `ssdt.cpp:55` | `EnumShadowSSDT()` | 枚举ShadowSSDT，处理数组[1]，SSDT_GetPfnAddr解码 |
| R3 | `ArkR3.cpp:785` | `ShadowSSDTGetVec()` | 获取ShadowSSDT数据，符号解析 |
| R3 | `ArkR3.cpp:777` | `GetWin32kFunctionName()` | Win32k符号解析，RVA计算 |
| UI | `KernelWnd.cpp` | `RenderShadowSSDTTable()` | UI显示，区分NtUser*/NtGdi*颜色 |

### 学习要点

**Windows内核架构**:
- ShadowSSDT 是图形子系统的系统调用表
- GUI进程可访问完整的KeServiceDescriptorTableShadow[0,1]
- 普通进程只能访问KeServiceDescriptorTable[0]

**反Rootkit技术**:
- ShadowSSDT Hook检测：对比原始地址vs当前地址
- Win32k模块完整性验证：通过PDB符号验证函数地址
- 双表监控：SSDT + ShadowSSDT 全覆盖系统调用

# 对象回调 (Object Callback) 实现技术说明

## 完成状态：✅ 已实现

### 核心技术难点解决

**1. 对象回调枚举原理**
```cpp
// 数据流动链路：
ObTypeIndexTable[] → _OBJECT_TYPE → CallbackList → CALLBACK_BODY → Pre/PostCallbackRoutine
```

**2. 关键数据结构理解**
```cpp
typedef struct _CALLBACK_NODE {
    USHORT Version;                 // 版本号，目前是0x100
    USHORT CallbackBodyCount;       // 本节点上CallbackBody的数量
    PVOID Context;                  // 注册回调时设定的RegistrationContext
    UNICODE_STRING Altitude;        // 指向Altitude字符串
    char CallbackBody[1];           // CALLBACK_BODY数组，元素个数为CallbackBodyCount  
} CALLBACK_NODE, *PCALLBACK_NODE;

typedef struct _OB_CALLBACK_REGISTRATION {
    USHORT Version;                 // 用户注册时的输入结构
    USHORT OperationRegistrationCount;
    UNICODE_STRING Altitude;
    PVOID RegistrationContext;
    OB_OPERATION_REGISTRATION* OperationRegistration;
} OB_CALLBACK_REGISTRATION, *POB_CALLBACK_REGISTRATION;
```

**3. 枚举与删除的数据转换**
- **注册阶段**: `OB_CALLBACK_REGISTRATION` (用户输入) → `CALLBACK_NODE` (内核存储)
- **枚举阶段**: `CALLBACK_BODY.CallbackNode` → `CALLBACK_INFO.CallbackRegistration`
- **删除阶段**: `ObUnRegisterCallbacks(CallbackNode)` → 从CallbackList移除整个注册

### 实现细节

**枚举实现 (callback.cpp:106-202)**
```cpp
// 1. 遍历ObTypeIndexTable获取所有对象类型
for (int i = 0; i < 100; i++) {
    ULONG_PTR objTypeAddr = *(ULONG_PTR*)(ObTypeIndexTable + i * sizeof(ULONG_PTR));
    
    // 2. 获取_OBJECT_TYPE->CallbackList链表头
    size_t CallbackListOffset = ntos.GetOffset("_OBJECT_TYPE", "CallbackList");
    LIST_ENTRY* head = (LIST_ENTRY*)(objTypeAddr + CallbackListOffset);
    
    // 3. 遍历链表，每个节点按CALLBACK_BODY解析
    CALLBACK_BODY* cb = CONTAINING_RECORD(entry, CALLBACK_BODY, ListEntry);
    
    // 4. 分别处理PreOp和PostOp回调
    if (cb->PreCallbackRoutine) {
        info->Extra.ObjectExtra.CallbackRegistration = cb->CallbackNode; // 关键：保存删除句柄
        // 名称格式：PreOb_Process, PreOb_Thread...
    }
    if (cb->PostCallbackRoutine) {
        info->Extra.ObjectExtra.CallbackRegistration = cb->CallbackNode;
        // 名称格式：PostOb_Process, PostOb_Thread...
    }
}
```

**删除实现 (callback.cpp:506-512)**
```cpp
case TypeObject:
{
    PVOID callbackRegistration = deleteKey; // 实际是CallbackNode
    ObUnRegisterCallbacks(callbackRegistration);
    Log("[XM] 删除对象回调，CallbackRegistration=%p", callbackRegistration);
    return STATUS_SUCCESS;
}
```

### 数据结构设计

**CALLBACK_INFO扩展 (proto.h:256-267)**
```cpp
struct {
    CHAR ObjectTypeName[32];        // "PreOb_Process" / "PostOb_Thread"
    PVOID ObjTypeAddr;              // 对象类型地址
    PVOID CallbackRegistration;     // CallbackNode，删除时传递给ObUnRegisterCallbacks
}ObjectExtra;
```

### 学习要点

**Windows内核架构**:
- 对象回调是Windows对象管理子系统的核心安全机制
- 每种对象类型(Process、Thread、Desktop等)都可以注册回调
- 回调分为PreOperation和PostOperation两个阶段

**反Rootkit技术**:
- 对象回调Hook检测：枚举所有对象类型的回调函数
- 恶意回调识别：通过模块路径和签名验证回调合法性
- 回调删除/恢复：通过ObUnRegisterCallbacks删除恶意回调注册
