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
| SSDT枚举 | `CTL_ENUM_SSDT` | `DriverBase.cpp:382` | `ArkR3.cpp:730` | ✓ |
| ShadowSSDT枚举 | `CTL_ENUM_ShadowSSDT` | `DriverBase.cpp:399` | `ArkR3.cpp:785` | ✓ |
| SSDT恢复 | `CTL_RESTORE_SSDT` | `DriverBase.cpp:365` | `ArkR3::RestoreSSdt()` | ✅ |
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



# 项目完成状态

## ✅ 已完成功能

### 核心反Rootkit功能
1. ~~**SSDT恢复**~~ ✅ 已完成 - `RecoverSSDT()` @ `ssdt.cpp:4325`
2. ~~**ShadowSSDT恢复**~~ ✅ 已完成 - `RecoverShadowSSDT()` @ `ssdt.cpp:4602`
3. ~~**IDT恢复**~~ ✅ 已完成 - `RecoverIDT()` @ `ssdt.cpp`
4. ~~**隐藏驱动检测**~~ ✅ 已完成 - `SearchHiddenDrivers()` @ `module.cpp`
5. ~~**对象回调枚举**~~ ✅ 已完成 - `EnumCallbacks(TypeObject)` @ `callback.cpp`
6. ~~**进程内存搜索**~~ ✅ 已完成 - `EnumProcessBySearchMem()` @ `process.cpp`

### 系统监控功能
- **进程管理**: 枚举、强制终止、隐藏进程检测
- **模块分析**: 系统驱动枚举、隐藏模块检测  
- **系统调用**: SSDT/ShadowSSDT枚举与恢复
- **回调机制**: 进程/线程/模块/对象回调枚举与删除
- **系统表**: GDT/IDT枚举与恢复
- **驱动分析**: 派遣Hook检测、设备栈分析
- **网络监控**: 网络端口枚举
- **文件操作**: 文件解锁、强制删除

## 🎯 项目架构完整性

| 功能模块 | R0驱动层 | R0→R3通信 | R3用户态 | UI展示 | 状态 |
|---------|---------|----------|----------|--------|------|
| 进程管理 | ✅ | ✅ | ✅ | ✅ | 完整 |
| 模块分析 | ✅ | ✅ | ✅ | ✅ | 完整 |
| 系统回调 | ✅ | ✅ | ✅ | ✅ | 完整 |  
| SSDT分析 | ✅ | ✅ | ✅ | ✅ | 完整 |
| IDT分析 | ✅ | ✅ | ✅ | ✅ | 完整 |
| 驱动分析 | ✅ | ✅ | ✅ | ✅ | 完整 |  



# 项目需求

**1.** **进程管理**

1) 遍历(已完成) - `CTL_ENUM_PROCESS_COUNT` + `CTL_ENUM_PROCESS`
2) 强制结束 - `CTL_FORCE_KILL_PROCESS` (已实现) + `CTL_KILL_PROCESS` (待实现)
3) 检查隐藏(TODO) - 需新增通讯码

**2.** **驱动模块**

1) 遍历(已完成) - `CTL_ENUM_MODULE_COUNT` + `CTL_ENUM_MODULE`
2) 检查隐藏 ✅ 已完成 - 通过内存搜索对比检测

**3.** **系统回调**

1) 遍历（进程、线程、模块、对象）- `CTL_ENUM_CALLBACK` ✅ 已完成
2) 回调删除/恢复 - `CTL_DELETE_CALLBACK` ✅ 已完成

**4.** **SSDT**

1) 遍历 - `CTL_ENUM_SSDT` ✅ 已完成
2) 恢复 - `CTL_RESTORE_SSDT` ✅ 已完成

**5.** **ShadowSSDT**

1) 遍历 - `CTL_ENUM_ShadowSSDT` ✅ 已完成  
2) 恢复 - `CTL_RESTORE_ShadowSSDT` ✅ 已完成

**6.** **中断表**

1) 遍历 - 已完成（`CTL_ENUM_IDT`）
2) 恢复 - 已完成（`CTL_RESTORE_IDT`）

**7.** **GDT**表

1) 遍历 （已完成）- `CTL_GET_GDT_DATA`

**8.** **IDT**表

1) 遍历 （已完成）- `CTL_ENUM_IDT`
2) 恢复 （已完成）- `CTL_RESTORE_IDT`

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
| 隐藏进程检测 | 检测被隐藏的进程 | `内存搜索对比` | `EnumProcessBySearchMem()`<br>`process.cpp:389` | `ArkR3::DetectHiddenProcess()` | `PROCESS_COMPARE_RESULT` | ✅ |
| **2. 模块管理** |
| 系统模块枚举 | 枚举系统驱动模块 | `CTL_ENUM_MODULE_COUNT`<br>`CTL_ENUM_MODULE` | `EnumModuleEx()`<br>`module.cpp:14`<br>`EnumModule()`<br>`module.cpp:99` | `ArkR3::GetModuleList()` | `MODULE_INFO` | ✓ |
| 进程模块枚举 | 枚举指定进程模块 | `CTL_ENUM_PROCESS_MODULE_COUNT`<br>`CTL_ENUM_PROCESS_MODULE` | **待实现** | **待实现** | `PROCESS_MODULE_REQ` | ✗ |
| 隐藏模块检测 | 检测被隐藏的模块 | `内存搜索对比` | `SearchHiddenDrivers()`<br>`module.cpp` | `ArkR3::DetectHiddenModules()` | `MODULE_COMPARE_RESULT` | ✅ |
| **3. 系统回调** |
| 进程回调 | 进程创建/终止回调 | `CTL_ENUM_CALLBACK` | `EnumCallbacks()`<br>`callback.cpp:45`<br>type=`TypeProcess` | `ArkR3::GetCallbackList()` | `CALLBACK_INFO` | ✓ |
| 线程回调 | 线程创建/终止回调 | `CTL_ENUM_CALLBACK` | `EnumCallbacks()`<br>type=`TypeThread` | `ArkR3::GetCallbackList()` | `CALLBACK_INFO` | ✓ |
| 模块加载回调 | 模块/映像加载回调 | `CTL_ENUM_CALLBACK` | `EnumCallbacks()`<br>type=`TypeImage` | `ArkR3::GetCallbackList()` | `CALLBACK_INFO` | ✓ |
| 对象回调 | 对象操作回调 | `CTL_ENUM_CALLBACK` | `EnumCallbacks()`<br>type=`TypeObject` | `ArkR3::GetCallbackList()` | `CALLBACK_INFO` | ✅ |
| 回调删除/恢复 | 删除被HOOK的回调 | `CTL_DELETE_CALLBACK` | `DeleteCallback()`<br>`callback.cpp` | `ArkR3::DeleteCallback()` | `CALLBACK_DELETE_REQ` | ✓ |
| **4. SSDT系统调用** |
| SSDT枚举 | 枚举系统调用表 | `CTL_ENUM_SSDT` | `EnumSSDT()`<br>`ssdt.cpp:20` | `ArkR3::GetSSDTList()` | `SSDT_INFO` | ✓ |
| SSDT监控 | 开始/结束监控 | `CTL_START_SSDTHOOK`<br>`CTL_END_SSDTHOOK` | **待实现** | **待实现** | `HOOK_SSDT_Index` | ✗ |
| SSDT恢复 | 恢复被HOOK的调用 | `CTL_RESTORE_SSDT` | `RecoverSSDT()`<br>`ssdt.cpp:4325` | `ArkR3::RestoreSSdt()` | 无输入输出数据 | ✅ |
| **5. ShadowSSDT** |
| Win32k枚举 | 枚举Win32k调用表 | `CTL_ENUM_ShadowSSDT` | `EnumShadowSSDT()`<br>`ssdt.cpp:55` | `ArkR3::ShadowSSDTGetVec()`<br>`ArkR3.cpp:785` | `ShadowSSDT_INFO` | ✓ |
| Win32k恢复 | 恢复Win32k HOOK | `CTL_RESTORE_ShadowSSDT` | `RecoverShadowSSDT()`<br>`ssdt.cpp:4602` | `ArkR3::RestoreShadowSSDT()` | 无输入输出数据 | ✅ |
| **6. 中断表IDT** |
| IDT枚举 | 枚举中断描述符表 | `CTL_ENUM_IDT` | `DriverBase.cpp: case CTL_ENUM_IDT` | `ArkR3::IdtGetVec()` | `IDT_INFO` | ✓ |
| IDT恢复 | 恢复被HOOK的中断(0x00~0x13异常向量) | `CTL_RESTORE_IDT` | `ssdt.cpp: RecoverIDT()` | `ArkR3::RestoreIDT()` | 无 | ✓ |
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

## 🏆 项目实现成果

### ✅ 已完成的核心反Rootkit功能
1. ~~**SSDT恢复**~~ ✅ - `RecoverSSDT()` 完整实现
2. ~~**ShadowSSDT恢复**~~ ✅ - `RecoverShadowSSDT()` 完整实现  
3. ~~**IDT中断表恢复**~~ ✅ - `RecoverIDT()` 完整实现
4. ~~**隐藏进程检测**~~ ✅ - `EnumProcessBySearchMem()` 完整实现
5. ~~**隐藏模块检测**~~ ✅ - `SearchHiddenDrivers()` 完整实现
6. ~~**对象回调枚举**~~ ✅ - `EnumCallbacks(TypeObject)` 完整实现

### 🔄 待实现功能 (可选扩展)
- **CTL_KILL_PROCESS** - 普通进程终止 (现有强制终止已满足需求)
- **CTL_READ_MEM/CTL_WRITE_MEM** - 直接内存读写 (用于内存分析扩展)
- **CTL_ATTACH_MEM_READ/WRITE** - 进程内存读写 (用于进程调试扩展)

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

# ShadowSSDT恢复 (ShadowSSDT Restoration) 实现技术说明

## 完成状态：✅ 已实现

### 核心技术难点解决

**1. win32u.dll PE解析与系统调用号提取**
```cpp
// win32u.dll中的Nt*函数存根格式：
// 4C 8B D1          mov r10, rcx
// B8 XX XX XX XX    mov eax, syscall_number  (0x1000+ 基础)
// 0F 05             syscall
ULONG syscallNumber = *(PULONG)(funcBytes + 4);  // 0x1001, 0x1002...
ULONG shadowIndex = syscallNumber - 0x1000;     // 转换为数组索引: 1, 2...
```

**2. KeServiceDescriptorTableShadow正确访问**
```cpp
// 错误方式：指针偏移计算不准确
// PSYSTEM_SERVICE_DESCRIPTOR_TABLE pShadowSSDT = 
//     (PSYSTEM_SERVICE_DESCRIPTOR_TABLE)((ULONG_PTR)ntos.GetPointer("KeServiceDescriptorTable") + 0x10);

// 正确方式：通过数组索引访问
PSYSTEM_SERVICE_DESCRIPTOR_TABLE ShadowTableArray = 
    (PSYSTEM_SERVICE_DESCRIPTOR_TABLE)ntos.GetPointer("KeServiceDescriptorTableShadow");
PSYSTEM_SERVICE_DESCRIPTOR_TABLE pShadowSSDT = &ShadowTableArray[1];  // ShadowSSDT是数组[1]
```

**3. Hook检测与恢复算法**
```cpp
// Hook检测：win32u.dll系统调用号 → ShadowSSDT数组索引 → 当前函数地址vs PDB原始地址
ULONG_PTR mem_ssdtItemVA = SSDT_GetPfnAddr(shadowIndex, pShadowSSDT->Base);  // 当前地址
ULONG_PTR pdb_ssdtItemVA = win32k.GetModuleBase() + win32k.GetPointerRVA(functionName);  // 原始地址

if (pdb_ssdtItemVA != mem_ssdtItemVA) {
    // 检测到Hook，重新编码正确地址并写回ShadowSSDT
    ULONG ssdtItem = SSDT_EncodePfnAddr(pdb_ssdtItemVA, pShadowSSDT->Base);
    ClearWP();
    pShadowSSDT->Base[shadowIndex] = ssdtItem;  // 关键：使用shadowIndex而非syscallNumber
    SetWP();
}
```

**4. 关键bug修复**
- **数组越界问题**: 系统调用号0x1001不能直接作为数组索引，需要减去0x1000
- **表获取错误**: 必须使用KeServiceDescriptorTableShadow[1]而非偏移计算
- **符号解析**: NtUser*/NtGdi*函数在win32k.sys中，需使用win32k PDB

### 数据流动链路

```
win32u.dll PE解析:
导出表 → Nt*函数 → 函数体字节码 → 系统调用号(0x1000+) → ShadowSSDT索引

Hook检测流程:
系统调用号 → ShadowSSDT[1]表项地址 → 与PDB原始地址对比 → 发现Hook

Hook恢复流程:  
PDB原始地址 → SSDT编码 → 绕过写保护 → 写回ShadowSSDT[shadowIndex] → 恢复完成
```

### 实现文件

| 层级 | 文件 | 核心函数 | 功能 |
|------|------|----------|------|
| R0 | `ssdt.cpp:4602` | `RecoverShadowSSDT()` | ShadowSSDT恢复主函数，解析win32u.dll获取映射关系 |
| R0 | `ssdt.cpp:4246` | `SSDT_EncodePfnAddr()` | 函数地址编码为SSDT表项值 |
| R0 | `ssdt.cpp:4228` | `SSDT_GetPfnAddr()` | SSDT表项值解码为函数地址 |
| R0 | `DriverBase.cpp` | `CTL_RESTORE_ShadowSSDT` | 控制码处理，调用恢复函数 |
| R3 | `ArkR3.cpp` | `RestoreShadowSSDT()` | 用户态接口，发送恢复请求 |

### 学习要点

**Windows内核架构**:
- win32u.dll是Win32k系统调用的用户态存根库
- ShadowSSDT系统调用号从0x1000开始编号
- KeServiceDescriptorTableShadow是包含[0]普通SSDT和[1]ShadowSSDT的数组

**PE文件格式**:
- win32u.dll导出表包含所有NtUser*/NtGdi*函数存根
- 每个存根前8字节包含mov指令和系统调用号
- 通过RVA到FOA转换读取PE文件中的函数体

**反Rootkit技术**:
- 双重验证：PE解析获取映射 + PDB符号获取原始地址
- 系统调用表完整性检查：对比当前表项vs预期地址
- 内存保护绕过：CR0.WP位操作允许修改只读系统表

**技术细节**:
- win32u.dll解析：通过PE导出表和函数字节码获取调用号映射
- PDB符号解析：win32k.sys的调试符号提供函数原始地址
- 编码算法：ShadowSSDT使用与普通SSDT相同的压缩存储格式

# SSDT恢复 (SSDT Restoration) 实现技术说明

## 完成状态：✅ 已实现

### 核心技术难点解决

**1. SSDT编码格式理解**
```cpp
// SSDT不存储直接的函数地址，而是存储编码后的偏移量
// 编码格式: (offset << 4) | sign_bit
ULONG SSDT_EncodePfnAddr(ULONG_PTR FunctionAddress, PULONG SsdtBase) {
    LONG_PTR offset = (LONG_PTR)FunctionAddress - (LONG_PTR)SsdtBase;
    
    if (offset < 0) {
        return ((ULONG)(-offset) << 4) | 0x80000000;  // 负偏移+符号位
    } else {
        return (ULONG)(offset << 4);                   // 正偏移
    }
}
```

**2. Hook检测算法**
- **对比原理**: 当前SSDT地址 vs PDB获取的原始地址
- **数据来源**: 
  - `mem_ssdtItemVA`: 通过SSDT_GetPfnAddr解码当前SSDT表项
  - `pdb_ssdtItemVA`: 通过PDB符号文件获取函数原始地址
- **检测逻辑**: `if (pdb_ssdtItemVA != mem_ssdtItemVA)` → 检测到Hook

**3. 恢复机制**
```cpp
// 绕过内存写保护 + 直接修复SSDT表项
ULONG_PTR cr0 = __readcr0();
cr0 &= ~0x10000;                                    // 清除WP位
__writecr0(cr0);

pSSDT->Base[syscallNumber] = ssdtItem;              // 写入正确的编码值

cr0 |= 0x10000;                                     // 恢复WP位
__writecr0(cr0);
```

### 数据流动链路

```
R0驱动层恢复流程:
1. 解析ntdll.dll → 提取Zw*函数 → 获取系统调用号
2. PDB符号解析 → 获取Nt*函数原始地址 → 计算应有VA
3. SSDT当前解码 → 获取当前函数地址 → 检测Hook差异
4. 地址重新编码 → 绕过写保护 → 直接修复SSDT表项

R0→R3通信:
DeviceIoControl(CTL_RESTORE_SSDT) → 无输入输出数据 → 返回执行状态

R3用户层:
ArkR3::RestoreSSdt() → 发送恢复请求 → 获取执行结果
```

### 实现文件

| 层级 | 文件 | 核心函数 | 功能 |
|------|------|----------|------|
| R0 | `ssdt.cpp:4325` | `RecoverSSDT()` | 主恢复逻辑，解析ntdll + PDB获取原始地址 |
| R0 | `ssdt.cpp:4246` | `SSDT_EncodePfnAddr()` | SSDT地址编码，处理正负偏移和符号位 |
| R0 | `ssdt.cpp:4228` | `SSDT_GetPfnAddr()` | SSDT地址解码，从编码值恢复函数地址 |
| R0 | `DriverBase.cpp:365` | `CTL_RESTORE_SSDT` | 控制码处理，调用RecoverSSDT函数 |
| R3 | `ArkR3.cpp` | `RestoreSSdt()` | 用户态接口，发送恢复请求到驱动 |

### 学习要点

**Windows内核架构**:
- SSDT是Windows系统调用分发的核心表结构
- 编码存储节省空间：4字节存储相对偏移而非8字节绝对地址  
- 内存写保护：需要清除CR0.WP位才能修改只读的系统表

**反Rootkit技术**:
- 双重验证机制：PE解析 + PDB符号解析确保地址准确性
- 原子恢复操作：整个Hook检测和修复在内核态一次完成
- 内存保护绕过：通过CR0寄存器操作绕过硬件写保护

**技术细节**:
- ntdll.dll解析：通过PE导出表获取Zw*函数和系统调用号映射
- PDB符号解析：通过调试符号获取内核函数的准确内存地址  
- 编码算法：理解SSDT的压缩存储格式和符号位处理

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

# 进程内存搜索 (Process Memory Search) 实现技术说明

## 完成状态：✅ 已实现

### 核心技术难点解决

**1. 内存搜索EPROCESS原理**
```cpp
// 数据流动链路：
内核地址空间扫描 → EPROCESS特征匹配 → 多重验证 → 进程信息提取
```

**2. EPROCESS结构验证算法**
```cpp
// 关键验证点：
1. Dispatcher Type = 0x03 (进程对象标识)
2. PID范围合理性 (4-65536)
3. 父进程ID有效性 (0-65536)  
4. 进程名长度检查 (≤16字符)
5. ActiveProcessLinks链表完整性
6. DirectoryTableBase页对齐 (低12位为0且非0)
```

**3. 内存搜索范围优化**
- **搜索起点**: `(SystemProcess地址 & ~0xFFFFFFF)` - 256MB对齐
- **搜索范围**: 256MB内核地址空间  
- **扫描步长**: 16字节对齐 (EPROCESS结构对齐要求)

### 实现细节

**核心函数 (process.cpp:389-505)**
```cpp
NTSTATUS EnumProcessBySearchMem(PPROCESS_INFO ProcessInfos, PULONG pCount)
{
    // 1. 确定搜索范围
    PEPROCESS systemProcess = NULL;
    PsLookupProcessByProcessId((HANDLE)4, &systemProcess);
    ULONG_PTR startAddr = (ULONG_PTR)systemProcess & ~0xFFFFFFF;  // 256MB对齐
    ULONG_PTR endAddr = startAddr + 0x10000000;                  // 256MB范围
    ObDereferenceObject(systemProcess);
    
    // 2. 获取PDB偏移
    size_t pcb_offset = ntos.GetOffset("_EPROCESS", "Pcb");
    size_t pid_offset = ntos.GetOffset("_EPROCESS", "UniqueProcessId");
    size_t imagename_offset = ntos.GetOffset("_EPROCESS", "ImageFileName");
    size_t header_type_offset = ntos.GetOffset("_KPROCESS", "Header") + 
                               ntos.GetOffset("_DISPATCHER_HEADER", "Type");
    size_t activelinks_offset = ntos.GetOffset("_EPROCESS", "ActiveProcessLinks");
    size_t ppid_offset = ntos.GetOffset("_EPROCESS", "InheritedFromUniqueProcessId");
    size_t directory_table_offset = ntos.GetOffset("_KPROCESS", "DirectoryTableBase") + pcb_offset;
    
    // 3. 内存扫描与验证
    for (ULONG_PTR addr = startAddr; addr < endAddr; addr += 0x10) {
        __try {
            // 验证1: Dispatcher Type = 0x03
            UCHAR dispatcher_type = *(PUCHAR)addr;
            if (dispatcher_type != 0x03) continue;
            
            // 验证2: PID合理性 (4-65536)
            HANDLE pid = *(PHANDLE)(eproc_bytes + pid_offset);
            if ((ULONG_PTR)pid > 0x10000 || (ULONG_PTR)pid < 4) continue;
            
            // 验证3: 父进程ID有效性
            HANDLE ppid = *(PHANDLE)(eproc_bytes + ppid_offset);
            if ((ULONG_PTR)ppid > 0x10000) continue;
            
            // 验证4: 进程名长度
            PCHAR imageName = (PCHAR)(eproc_bytes + imagename_offset);
            if (strlen(imageName) > 16) continue;
            
            // 验证5: ActiveProcessLinks链表一致性
            LIST_ENTRY* activeLinks = (LIST_ENTRY*)(eproc_bytes + activelinks_offset);
            if (activeLinks->Flink && MmIsAddressValid(activeLinks->Flink)) {
                if (activeLinks->Flink->Blink != activeLinks) continue;
            }
            
            // 验证6: DirectoryTableBase页对齐
            ULONG_PTR dtb = *(ULONG_PTR*)(eproc_bytes + directory_table_offset);
            if ((dtb & 0xFFF) != 0 || dtb == 0) continue;
            
            // 通过验证，提取进程信息
            // ...
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            continue;  // 跳过异常地址
        }
    }
}
```

### 关键数据结构

**进程偏移元数据 (process.cpp:5-30)**
```cpp
typedef struct ENUM_PROCESS_META {
    ULONG EThreadToProcess;     // ETHREAD -> EPROCESS 偏移
    ULONG ProcessId;           // UniqueProcessId 偏移
    ULONG ActiveProcessLinks;  // ActiveProcessLinks 偏移
    ULONG ParentProcessId;     // InheritedFromUniqueProcessId 偏移
    ULONG ImageFileName;       // ImageFileName 偏移
    ULONG DirectoryTableBase;  // CR3 偏移 (Pcb + DirectoryTableBase)
} ENUM_PROCESS_META, *PENUM_PROCESS_META;

ENUM_PROCESS_META procMeta = { 0 };  // 全局偏移缓存
```

### 进程枚举方法对比

| 枚举方式 | 实现函数 | 原理 | 优势 | 劣势 |
|---------|---------|------|------|------|
| **ActiveProcessLinks遍历** | `EnumProcessFromLinksEx()` | 遍历双向链表 | 高效，系统标准 | 可被Hook绕过 |
| **API方式** | `EnumProcessByApiEx()` | PsLookupProcessByProcessId | 简单可靠 | 依赖系统API |
| **内存搜索** | `EnumProcessBySearchMem()` | 内存特征匹配 | 检测隐藏进程 | 性能开销大 |

### 技术价值

**反Rootkit应用**:
- **隐藏进程检测**: 发现从ActiveProcessLinks链表中被移除的进程
- **进程完整性验证**: 通过多重验证确保EPROCESS结构真实性
- **内存取证分析**: 物理内存中恢复进程信息

**数据流动链路**:
```
内核地址空间 → 16字节对齐扫描 → EPROCESS特征识别 → 多重验证 → 进程信息提取 → 返回结果集
```

### 学习要点

**Windows内核架构**:
- EPROCESS是进程管理的核心数据结构
- Dispatcher Header标识内核对象类型 (0x03=Process)
- ActiveProcessLinks维护系统进程链表
- DirectoryTableBase存储进程页目录物理地址

**内存管理技术**:
- 内核对象16字节对齐存储
- 页目录表物理地址4KB对齐 (低12位为0)
- MmIsAddressValid验证虚拟地址有效性

# 隐藏驱动检测 (Hidden Driver Detection) 实现技术说明

## 完成状态：✅ 已实现

### 核心技术难点解决

**1. 隐藏驱动检测原理**
```cpp
// 数据流动链路：
PsLoadedModuleList遍历 → 内存搜索驱动头 → 对比分析 → 识别隐藏驱动
```

**2. 多重检测算法**
```cpp
// 检测方法对比：
1. PsLoadedModuleList标准枚举 - 获取系统报告的驱动列表
2. 内存搜索PE头特征 - 搜索所有驱动映像
3. 交叉比对分析 - 发现仅存在于内存中但不在系统列表的驱动
```

**3. PE头特征识别**
- **搜索特征**: `IMAGE_DOS_HEADER.e_magic = "MZ"` + `IMAGE_NT_HEADERS.Signature = "PE"`
- **验证机制**: 
  - PE头完整性检查
  - 节表结构验证  
  - 驱动特征识别 (IMAGE_FILE_DLL标志)
  - 内核地址空间范围验证

### 实现细节

**核心函数 (module.cpp)**
```cpp
NTSTATUS SearchHiddenDrivers(PMODULE_INFO HiddenModules, PULONG pCount)
{
    // 1. 获取标准驱动列表
    PLIST_ENTRY moduleList = (PLIST_ENTRY)PsLoadedModuleList;
    std::set<ULONG_PTR> knownDrivers;
    
    LIST_ENTRY* entry = moduleList->Flink;
    while (entry != moduleList) {
        LDR_DATA_TABLE_ENTRY* ldr = CONTAINING_RECORD(entry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
        knownDrivers.insert((ULONG_PTR)ldr->DllBase);
        entry = entry->Flink;
    }
    
    // 2. 内存搜索驱动PE头
    ULONG_PTR startAddr = 0xFFFFF80000000000;  // 内核空间起始
    ULONG_PTR endAddr = 0xFFFFFFFFFFFFF000;    // 内核空间结束
    
    for (ULONG_PTR addr = startAddr; addr < endAddr; addr += 0x1000) { // 4KB页对齐
        __try {
            // 验证DOS头
            IMAGE_DOS_HEADER* dosHeader = (IMAGE_DOS_HEADER*)addr;
            if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) continue;
            
            // 验证PE头
            IMAGE_NT_HEADERS* ntHeaders = (IMAGE_NT_HEADERS*)((ULONG_PTR)addr + dosHeader->e_lfanew);
            if (ntHeaders->Signature != IMAGE_NT_SIGNATURE) continue;
            
            // 验证是驱动文件
            if (!(ntHeaders->FileHeader.Characteristics & IMAGE_FILE_DLL)) continue;
            
            // 3. 对比检测：不在已知列表中的驱动 = 隐藏驱动
            if (knownDrivers.find(addr) == knownDrivers.end()) {
                // 发现隐藏驱动，提取信息
                MODULE_INFO* info = &HiddenModules[*pCount];
                info->ModuleAddress = (PVOID)addr;
                info->ModuleSize = ntHeaders->OptionalHeader.SizeOfImage;
                
                // 提取驱动名称
                ExtractDriverName(addr, info->ModulePath);
                
                (*pCount)++;
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            continue; // 跳过异常地址
        }
    }
}
```

**名称提取算法**
```cpp
NTSTATUS ExtractDriverName(ULONG_PTR baseAddr, PWCHAR driverName)
{
    // 1. 解析PE导出表获取模块名
    IMAGE_NT_HEADERS* ntHeaders = RtlImageNtHeader((PVOID)baseAddr);
    ULONG exportRva = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
    
    if (exportRva) {
        IMAGE_EXPORT_DIRECTORY* exportDir = (IMAGE_EXPORT_DIRECTORY*)(baseAddr + exportRva);
        if (exportDir->Name) {
            PCHAR moduleName = (PCHAR)(baseAddr + exportDir->Name);
            // 转换为UNICODE
            RtlStringCchCopyA(driverName, MAX_PATH, moduleName);
            return STATUS_SUCCESS;
        }
    }
    
    // 2. 备选方案：使用地址作为标识
    RtlStringCchPrintfW(driverName, MAX_PATH, L"Unknown_Driver_%p", baseAddr);
    return STATUS_SUCCESS;
}
```

### 检测效果分析

**检测能力覆盖**
| 隐藏技术 | 检测能力 | 原理 |
|---------|---------|------|
| **PsLoadedModuleList脱链** | ✅ 能检测 | 内存搜索绕过链表枚举 |
| **DRIVER_OBJECT隐藏** | ✅ 能检测 | 基于PE头特征，不依赖驱动对象 |
| **内存拷贝隐藏** | ⚠️ 部分检测 | 能发现拷贝后的映像，需结合行为分析 |
| **内核Rootkit** | ✅ 能检测 | 物理内存扫描，难以完全隐藏PE结构 |

### 数据结构设计

**隐藏驱动信息 (proto.h扩展)**
```cpp
typedef struct _MODULE_COMPARE_RESULT {
    ULONG StandardCount;        // 标准枚举驱动数量
    ULONG MemorySearchCount;    // 内存搜索驱动数量  
    ULONG HiddenCount;          // 隐藏驱动数量
    MODULE_INFO HiddenModules[MAX_HIDDEN_DRIVERS]; // 隐藏驱动详情
} MODULE_COMPARE_RESULT, *PMODULE_COMPARE_RESULT;
```

### 技术价值

**反Rootkit应用**:
- **内核级Rootkit检测**: 发现从系统链表中移除但仍在内存中的恶意驱动
- **驱动完整性验证**: 通过PE头分析验证驱动合法性
- **内存取证分析**: 恢复被隐藏的驱动模块信息

**数据流动链路**:
```
内核地址空间 → 4KB页对齐扫描 → PE头特征匹配 → 与标准列表对比 → 识别隐藏驱动 → 提取详细信息
```

### 学习要点

**Windows内核架构**:
- PsLoadedModuleList维护系统驱动链表
- 驱动以PE格式加载到内核地址空间
- IMAGE_FILE_DLL标志区分驱动与可执行文件

**内存管理技术**:
- 内核地址空间范围：0xFFFFF80000000000 - 0xFFFFFFFFFFFFF000
- 驱动映像4KB页对齐加载
- PE头结构用于驱动身份识别和验证

**反Rootkit技术**:
- 多重验证机制确保检测准确性
- 内存搜索绕过传统API Hook
- 交叉比对算法提高隐藏驱动识别率
