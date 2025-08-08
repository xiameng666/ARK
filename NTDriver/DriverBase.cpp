#include "DriverBase.h"

#define MAX_OBJECT_STORE 500                     // 最多存储500个驱动对象
extern DRIVER_OBJECT_INFO g_DrvObjs[MAX_OBJECT_STORE];
extern ULONG g_DrvObjCount;                         // 枚举得到的驱动数量

extern "C" void _sgdt(void*);
BOOLEAN g_LogOn = TRUE;
ULONG_PTR g_VA = 0;
PDRIVER_OBJECT g_DriverObject = NULL;  // 保存当前驱动对象

wchar_t g_ntosPdbPath[512] = L"C:\\Symbols";
wchar_t g_win32kPdbPath[512] = L"C:\\Symbols";

ULONG g_CoreCount = 0; //核心数
void* g_IdtData = NULL;  //保存的idt数据

void EnableWrite() {
    ULONG_PTR cr0 = __readcr0();
    cr0 &= ~0x10000; // 清除WP位
    __writecr0(cr0);
}
void DisableWrite() {
    ULONG_PTR cr0 = __readcr0();
    cr0 |= 0x10000;
    __writecr0(cr0);
}

NTSTATUS SetPdbPath(PWCHAR NtosPath,PWCHAR Win32kPath)
{
    if (!NtosPath || !Win32kPath) {
        return STATUS_INVALID_PARAMETER;
    }

    // 设置ntoskrnl PDB路径
    RtlZeroMemory(g_ntosPdbPath, sizeof(g_ntosPdbPath));
    RtlCopyMemory(g_ntosPdbPath, NtosPath, wcslen(NtosPath) * sizeof(WCHAR));

    // 设置win32k PDB路径
    RtlZeroMemory(g_win32kPdbPath, sizeof(g_win32kPdbPath));
    RtlCopyMemory(g_win32kPdbPath, Win32kPath, wcslen(Win32kPath) * sizeof(WCHAR));

    Log("[XM] SetPdbPaths - Ntos: %ws, Win32k: %ws", g_ntosPdbPath, g_win32kPdbPath);

    return STATUS_SUCCESS;
}

void Log(const char* Format, ...) {
    if (!g_LogOn) return;
    
    va_list args;
    va_start(args, Format);
    
    char buffer[512];
    RtlStringCbVPrintfA(buffer, sizeof(buffer), Format, args);
    
    DbgPrint("%s\n", buffer);
    va_end(args);
}

NTSTATUS CompleteRequest(struct _IRP* Irp, ULONG_PTR Information, NTSTATUS Status) {
    Irp->IoStatus.Status = Status;
    Irp->IoStatus.Information = Information;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return Status;
}

NTSTATUS DispatchCreate(_In_ struct _DEVICE_OBJECT* DeviceObject, _Inout_ struct _IRP* Irp) {
    UNREFERENCED_PARAMETER(DeviceObject);
    Log("[XM] DispatchCreate");
    return CompleteRequest(Irp);
}

NTSTATUS DispatchClose(_In_ struct _DEVICE_OBJECT* DeviceObject, _Inout_ struct _IRP* Irp) {
    UNREFERENCED_PARAMETER(DeviceObject);
    Log("[XM] DispatchClose");
    return CompleteRequest(Irp);
}

NTSTATUS DispatchRead(_In_ struct _DEVICE_OBJECT* DeviceObject, _Inout_ struct _IRP* Irp) {
    UNREFERENCED_PARAMETER(DeviceObject);
    Log("[XM] DispatchRead");
    return CompleteRequest(Irp);
}

NTSTATUS DispatchWrite(_In_ struct _DEVICE_OBJECT* DeviceObject, _Inout_ struct _IRP* Irp) {
    UNREFERENCED_PARAMETER(DeviceObject);
    
    PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
    ULONG length = stack->Parameters.Write.Length;
    
    if (length == sizeof(ULONG_PTR)) {
        PULONG_PTR pData = (PULONG_PTR)Irp->AssociatedIrp.SystemBuffer;
        if (pData) {
            g_VA = *pData;
            Log("[XM] DispatchWrite: VA=0x%p", (PVOID)g_VA);
            return CompleteRequest(Irp, sizeof(ULONG_PTR));
        }
    }
    
    Log("[XM] DispatchWrite:  %d", length);
    return CompleteRequest(Irp, 0, STATUS_INVALID_PARAMETER);
}

NTSTATUS DispatchDeviceControl(_In_ struct _DEVICE_OBJECT* DeviceObject, _Inout_ struct _IRP* Irp) {
    UNREFERENCED_PARAMETER(DeviceObject);
    
    PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
    ULONG controlCode = stack->Parameters.DeviceIoControl.IoControlCode;
    NTSTATUS status = STATUS_INVALID_DEVICE_REQUEST;
    ULONG_PTR info = 0;

    switch (controlCode) {
        case CTL_SET_PDB_PATH: 
        {
            __try {
                PPDB_PATH_REQUEST pdbPathReq = (PPDB_PATH_REQUEST)Irp->AssociatedIrp.SystemBuffer;
                SetPdbPath(pdbPathReq->NtosPath, pdbPathReq->Win32kPath);
                status = STATUS_SUCCESS;
                Log("[XM] CTL_SET_PDB_PATH: %ws   %ws ", g_ntosPdbPath,g_win32kPdbPath);
                //KdBreakPoint();
                InitProcessPdb();
                //ForTest();

            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                status = STATUS_UNSUCCESSFUL;
                Log("[XM] CTL_SET_PDB_PATH exception");
            }
            break;
        }

        case CTL_ENUM_DRIVER_OBJECT: {
            __try {
                EnumDriverObject();  // 刷新g_DrvObjs[]数据                  
                ULONG copySize = g_DrvObjCount * sizeof(DRIVER_OBJECT_INFO);
                RtlCopyMemory(Irp->AssociatedIrp.SystemBuffer, g_DrvObjs, copySize);
                info = copySize;
                status = STATUS_SUCCESS;
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                status = STATUS_UNSUCCESSFUL;
                Log("[XM] CTL_ENUM_DRIVER_OBJECT exception");
            }
            break;
        }

         case CTL_SEND_SSDTBASE:
        {
            __try {
                Log("[XM] CTL_SEND_SSDTBASE: 开始处理");

                INIT_NTOS;

                ULONG_PTR kiServiceTableRVA = ntos.GetPointerRVA("KeServiceDescriptorTable");
                Log("[XM] KiServiceTable RVA查询结果: 0x%x", kiServiceTableRVA);

             
                // 写入SystemBuffer
                *(PULONG_PTR)Irp->AssociatedIrp.SystemBuffer = kiServiceTableRVA;
                info = sizeof(ULONG_PTR);
                status = STATUS_SUCCESS;

                Log("[XM] 成功返回RVA: 0x%x", kiServiceTableRVA);
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                ULONG exceptionCode = GetExceptionCode();
                status = STATUS_UNSUCCESSFUL;
                Log("[XM] CTL_SEND_SSDTBASE exception: 0x%x", exceptionCode);
            }
            break;
        }

        case CTL_GET_GDT_DATA:
        {
            __try {
                PGDT_DATA_REQ gdtReq = (PGDT_DATA_REQ)Irp->AssociatedIrp.SystemBuffer;
                ULONG cpuIndex = gdtReq->CpuIndex;
                GDTR gdtr = { 0 };
                
                KAFFINITY oldAffinity = KeSetSystemAffinityThreadEx(1ULL << cpuIndex);
                
                _sgdt(&gdtr);
                
                ULONG gdtSize = gdtr.Limit + 1;
                Log("[XM] CpuIndex: %d GdtBase: %p GdtLimit: %08x, Size: %d",
                    cpuIndex, (PVOID)gdtr.Base, gdtr.Limit, gdtSize);
                
                RtlCopyMemory(Irp->AssociatedIrp.SystemBuffer, (PVOID)gdtr.Base, gdtSize);
                info = gdtSize;
                status = STATUS_SUCCESS;
                
                KeRevertToUserAffinityThreadEx(oldAffinity);
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                status = STATUS_UNSUCCESSFUL;
                Log("[XM] CTL_GET_GDT_DATA exception");
            }
            break;
        }

        case CTL_ENUM_IDT_COUNT:
        {
            __try {
                ULONG totalIdtCount = 0;

                // 获取CPU数量
                ULONG cpuCount = KeQueryActiveProcessorCountEx(ALL_PROCESSOR_GROUPS);

                // 统计所有条目
                for (ULONG cpu = 0; cpu < cpuCount; cpu++) {
                    KAFFINITY oldAffinity = KeSetSystemAffinityThreadEx(1ULL << cpu);

                    IDTR idtr = { 0 };
                    __sidt(&idtr);

                    ULONG idtEntries = (idtr.Limit + 1) / sizeof(InterruptDescriptor);
                    totalIdtCount += idtEntries;

                    Log("cpuindex:%d base:%p size%d ", cpu, idtr.Base, idtEntries);

                    KeRevertToUserAffinityThreadEx(oldAffinity);
                }

                *(PULONG)Irp->AssociatedIrp.SystemBuffer = totalIdtCount;
                info = sizeof(ULONG);
                status = STATUS_SUCCESS;

                Log("[XM] CTL_ENUM_IDT_COUNT: %d", totalIdtCount);
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                status = STATUS_UNSUCCESSFUL;
                Log("[XM] CTL_ENUM_IDT_COUNT exception");
            }
            break;
        }

        case CTL_RESTORE_IDT:
        {
            __try {
                // 检查是否有保存的IDT数据
                if (g_CoreCount==0 || !g_IdtData) {
                    Log("[XM] 没有保存的IDT数据，无法恢复");
                    status = STATUS_INVALID_DEVICE_REQUEST;
                    break;
                }

                ULONG offset = 0;
                ULONG IdtSize = 0;
                for (ULONG cpu = 0; cpu < g_CoreCount; cpu++) {
                    KAFFINITY oldAffinity = KeSetSystemAffinityThreadEx(1ULL << cpu);

                    IDTR idtr = { 0 };
                    __sidt(&idtr);
                    IdtSize = idtr.Limit + 1;
                    offset = cpu * IdtSize;

                    EnableWrite();
                    RtlCopyMemory((void*)idtr.Base, (PUCHAR)g_IdtData + offset, IdtSize);
                    DisableWrite();

                    KeRevertToUserAffinityThreadEx(oldAffinity);
                }

                status = STATUS_SUCCESS;
                info = 0;
                Log("[XM] CTL_RESTORE_IDT 完成");
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                status = STATUS_UNSUCCESSFUL;
                Log("[XM] CTL_RESTORE_IDT exception");
            }
            break;
        }

        case CTL_ENUM_IDT:
        {
            __try {
                EnumModule();

                PIDT_INFO pIdtInfo = (PIDT_INFO)Irp->AssociatedIrp.SystemBuffer;
                ULONG currentIndex = 0;

                ULONG cpuCount = KeQueryActiveProcessorCountEx(ALL_PROCESSOR_GROUPS);

                {
                    //申请放得下所有idt表的内存 恢复的时候直接覆写
                    g_CoreCount = cpuCount;
                    IDTR idtr = { 0 };
                    __sidt(&idtr);

                    ULONG allocSize = g_CoreCount * (idtr.Limit + 1);
                    g_IdtData = ExAllocatePoolWithTag(NonPagedPool, allocSize, 'IDT0');
                    if (!g_IdtData) {
                        Log("[XM] IDT数据存储空间分配失败，大小=%d", allocSize);
                        return STATUS_INSUFFICIENT_RESOURCES;
                    }
                    RtlZeroMemory(g_IdtData, allocSize);
                }

                INIT_NTOS;

                for (ULONG cpu = 0; cpu < cpuCount; cpu++) {
                    KAFFINITY oldAffinity = KeSetSystemAffinityThreadEx(1ULL << cpu);

                    IDTR idtr = { 0 };
                    __sidt(&idtr);

                    ULONG idtSize = idtr.Limit + 1;
                    ULONG idtEntries = idtSize / sizeof(InterruptDescriptor);
                    PINTDESC pIdtBase = (PINTDESC)idtr.Base;

                    //保存IDT数据到数据结构  恢复的时候直接覆写
                    ULONG offset = cpu * idtSize;
                    RtlCopyMemory((PUCHAR)g_IdtData+ offset, pIdtBase, idtSize);

                    // 枚举所有IDT条目
                    for (ULONG i = 0; i < idtEntries; i++) {
                        PIDT_INFO pInfo = &pIdtInfo[currentIndex];
                        RtlZeroMemory(pInfo, sizeof(IDT_INFO));

                        // 基本信息
                        pInfo->CpuId = cpu;
                        pInfo->id = i;  
                        pInfo->Selector = pIdtBase[i].Selector;
                        pInfo->Dpl = pIdtBase[i].Dpl;
                        pInfo->Type = pIdtBase[i].Type;

                        // 地址重组
                        pInfo->Address = ((ULONG_PTR)pIdtBase[i].OffsetHigh << 32) |
                            ((ULONG_PTR)pIdtBase[i].OffsetMiddle << 16) |
                            pIdtBase[i].OffsetLow;

                    
                        // 查找地址所在模块
                        if (pIdtBase[i].Present && pInfo->Address != 0) {
                            CHAR modulePath[256] = { 0 };
                            NTSTATUS findStatus = FindModuleByAddress(
                                (PVOID)pInfo->Address,
                                modulePath,
                                NULL,
                                NULL
                            );

                            if (NT_SUCCESS(findStatus)) {
                                RtlStringCchCopyA(pInfo->Path, sizeof(pInfo->Path), modulePath);
                            }
                            else {
                                RtlStringCchCopyA(pInfo->Path, sizeof(pInfo->Path), "Unknown");
                            }
                            /*
                            //在内核模块中 获取函数名
                            __try {
                                auto name = ntos.GetNameByVA(pInfo->Address);
                                if (name != nullptr) {
                                    RtlStringCchCopyA(pInfo->funcName, sizeof(pInfo->funcName),
                                        name);
                                }
                               
                            }
                            __except (EXCEPTION_EXECUTE_HANDLER) {
                                Log("字符串替换有问题");
                            }
                            */
                            
                        }
                        else {
                            // Present=0 或地址为0的条目
                            RtlStringCchCopyA(pInfo->Path, sizeof(pInfo->Path), "Not Present");
                        }

                        currentIndex++;
                    }

                    KeRevertToUserAffinityThreadEx(oldAffinity);
                }

                info = currentIndex * sizeof(IDT_INFO);
                status = STATUS_SUCCESS;

                Log("[XM] CTL_ENUM_IDT: 返回 %d 个IDT条目", currentIndex);
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                status = STATUS_UNSUCCESSFUL;
                Log("[XM] CTL_ENUM_IDT exception");
            }
            break;
        }

        case CTL_ENUM_PROCESS_COUNT:
        {
            __try {
                //KdBreakPoint();
                ULONG processCount = 0;
                status = EnumProcessByApiEx(NULL, TRUE, &processCount);
                if (NT_SUCCESS(status)) {
                    *(PULONG)Irp->AssociatedIrp.SystemBuffer = processCount;
                    info = sizeof(ULONG);
                    Log("[XM] CTL_ENUM_PROCESS_COUNT: %d 个进程", processCount);
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                status = STATUS_UNSUCCESSFUL;
                Log("[XM] CTL_ENUM_PROCESS_COUNT exception");
            }
            break;
        }

        case CTL_MEMSEARCH_PROCESS:
        {
            __try {
                ULONG processCount = 0;
                status = EnumProcessBySearchMem((PPROCESS_INFO)Irp->AssociatedIrp.SystemBuffer, &processCount);
                if (NT_SUCCESS(status)) {
                    info = processCount * sizeof(PROCESS_INFO);
                    Log("[XM] CTL_ENUM_PROCESS: 获取 %d 个进程信息", processCount);
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                status = STATUS_UNSUCCESSFUL;
                Log("[XM] CTL_ENUM_PROCESS exception");
            }
            break;
        }
        case CTL_ENUM_PROCESS:
        {
            __try {
                ULONG processCount = 0;
                status = EnumProcessByApiEx((PPROCESS_INFO)Irp->AssociatedIrp.SystemBuffer, FALSE, &processCount);
                if (NT_SUCCESS(status)) {
                    info = processCount * sizeof(PROCESS_INFO);
                    Log("[XM] CTL_ENUM_PROCESS: 获取 %d 个进程信息", processCount);
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                status = STATUS_UNSUCCESSFUL;
                Log("[XM] CTL_ENUM_PROCESS exception");
            }
            break;
        }
        
        case CTL_ENUM_MODULE_COUNT:
        {
            __try {
                ULONG moduleCount = 0;
                status = EnumModuleEx(NULL, TRUE, &moduleCount);
                if (NT_SUCCESS(status)) {
                    *(PULONG)Irp->AssociatedIrp.SystemBuffer = moduleCount;
                    info = sizeof(ULONG);
                    Log("[XM] CTL_ENUM_MODULE_COUNT: %d 个模块", moduleCount);
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                status = STATUS_UNSUCCESSFUL;
                Log("[XM] CTL_ENUM_MODULE_COUNT exception");
            }
            break;
        }
        
        case CTL_ENUM_MODULE:
        {
            __try {
                ULONG moduleCount = 0;
                status = EnumModuleEx((PMODULE_INFO)Irp->AssociatedIrp.SystemBuffer, FALSE, &moduleCount);
                if (NT_SUCCESS(status)) {
                    info = moduleCount * sizeof(MODULE_INFO);
                    Log("[XM] CTL_ENUM_MODULE: 获取 %d 个模块信息", moduleCount);
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                status = STATUS_UNSUCCESSFUL;
                Log("[XM] CTL_ENUM_MODULE exception");
            }
            break;
        }

        case CTL_RESTORE_SSDT:
        {
            __try {
                status = RecoverSSDT();
                if (NT_SUCCESS(status)) {
                    Log("[XM] CTL_RESTORE_SSDT: SSDT恢复成功");
                }

                info = 0;                                       
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                status = STATUS_UNSUCCESSFUL;
                Log("[XM] CTL_RESTORE_SSDT exception");
            }
            break;
        }

        case CTL_RESTORE_SHADOW_SSDT:
        {
            __try {
                status = RecoverShadowSSDT();
                if (NT_SUCCESS(status)) {
                    Log("[XM] CTL_RESTORE_SHADOW_SSDT: Shadow恢复成功");
                }

                info = 0;
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                status = STATUS_UNSUCCESSFUL;
                Log("[XM] CTL_RESTORE_SHADOW_SSDT exception");
            }
            break;
        }

        case CTL_ENUM_SSDT:
        {
            __try {
                ULONG ssdtCount = 0;
                status = EnumSSDTFromMem((PSSDT_INFO)Irp->AssociatedIrp.SystemBuffer,&ssdtCount);
                if (NT_SUCCESS(status)) {
                    info = ssdtCount * sizeof(SSDT_INFO);
                    Log("[XM] CTL_ENUM_SSDT: 获取 %d 个SSDT条目", ssdtCount);
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                status = STATUS_UNSUCCESSFUL;
                Log("[XM] CTL_ENUM_SSDT exception");
            }
            break;
        }

        case CTL_ENUM_ShadowSSDT:
        {
            __try {
                ULONG ssdtCount = 0;
                status = EnumShadowSSDT((PSSDT_INFO)Irp->AssociatedIrp.SystemBuffer, &ssdtCount);
                if (NT_SUCCESS(status)) {
                    info = ssdtCount * sizeof(SSDT_INFO);
                    Log("[XM] CTL_ENUM_ShadowSSDT: 获取 %d 个ShadowSSDT条目", ssdtCount);
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                status = STATUS_UNSUCCESSFUL;
                Log("[XM] CTL_ENUM_ShadowSSDT exception");
            }
            break;
        }

        //case CTL_ENUM_CALLBACK_COUNT: 不需要了 因为R3端无法缓存回调数据。
       
        case CTL_ENUM_CALLBACK:
        {
            __try {
                // R3端发送回调类型，R0端返回该类型的所有回调信息
                PULONG callbackType = (PULONG)Irp->AssociatedIrp.SystemBuffer;
                ULONG callbackCount = 0;
                
                status = EnumCallbacks((PCALLBACK_INFO)Irp->AssociatedIrp.SystemBuffer, 
                                     (CALLBACK_TYPE)*callbackType, 
                                     &callbackCount);
                                     
                if (NT_SUCCESS(status)) {
                    info = callbackCount * sizeof(CALLBACK_INFO);
                    Log("[XM] CTL_ENUM_CALLBACK: 类型 %d 获取 %d 个回调信息", *callbackType, callbackCount);
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                status = STATUS_UNSUCCESSFUL;
                Log("[XM] CTL_ENUM_CALLBACK exception");
            }
            break;
        }
        
        case CTL_DELETE_CALLBACK:
        {
            __try {
                PCALLBACK_DELETE_REQ restoreReq = (PCALLBACK_DELETE_REQ)Irp->AssociatedIrp.SystemBuffer;
                
                status = DeleteCallback(restoreReq->Type, restoreReq->Index, restoreReq->CallbackFuncAddr);
                
                if (NT_SUCCESS(status)) {
                    Log("[XM] CTL_DELETE_CALLBACK: 成功删除回调 类型=%d 索引=%d", 
                        restoreReq->Type, restoreReq->Index);
                } 
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                status = STATUS_UNSUCCESSFUL;
                Log("[XM] CTL_DELETE_CALLBACK exception");
            }
            break;
        }
        
        case CTL_ENUM_DISPATCH_HOOK:
        {
            __try {

                ULONG hookCount = 0;
                status = EnumDrvMJHooked((PDISPATCH_HOOK_INFO)Irp->AssociatedIrp.SystemBuffer, &hookCount);
                
                if (NT_SUCCESS(status)) {
                    info = hookCount * sizeof(DISPATCH_HOOK_INFO);
                    Log("[XM] CTL_ENUM_DISPATCH_HOOK: 检测到 %d 个派遣函数信息", hookCount);
                } 
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                status = STATUS_UNSUCCESSFUL;
                Log("[XM] CTL_ENUM_DISPATCH_HOOK exception");
            }
            break;
        }
        
        case CTL_ENUM_DEVICE_STACK:
        {
            __try {
                ULONG stackCount = 0;
                status = EnumDeviceStackAttach((PDEVICE_STACK_INFO)Irp->AssociatedIrp.SystemBuffer, &stackCount);
                
                if (NT_SUCCESS(status)) {
                    info = stackCount * sizeof(DEVICE_STACK_INFO);
                    Log("[XM] CTL_ENUM_DEVICE_STACK: 分析了 %d 个设备栈", stackCount);
                } 
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                status = STATUS_UNSUCCESSFUL;
                Log("[XM] CTL_ENUM_DEVICE_STACK exception");
            }
            break;
        }
        
        case CTL_ENUM_NETWORK_PORT://暂时没用 在R3实现了
        {
            __try {
                //后续可能需要R0提供路径 先保留
                ULONG portCount = 0;
                status = EnumNetworkPort((PNETWORK_PORT_INFO)Irp->AssociatedIrp.SystemBuffer, &portCount);
                
                if (NT_SUCCESS(status)) {
                    info = portCount * sizeof(NETWORK_PORT_INFO);
                } 
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                status = STATUS_UNSUCCESSFUL;
                Log("[XM] CTL_ENUM_NETWORK_PORT exception");
            }
            break;
        }

        case CTL_FORCE_KILL_PROCESS:
        {
            ULONG PID = *(PULONG)Irp->AssociatedIrp.SystemBuffer;
            status = TerminateProcessByThread((HANDLE)(ULONG_PTR)PID);

            if (NT_SUCCESS(status)) {
                Log("[XM] Success terminate process %d", (ULONG)PID);
            }
            break;
        }

        case CTL_UNLOCK_FILE:
        {
            __try {
                PFILE_REQ req = (PFILE_REQ)Irp->AssociatedIrp.SystemBuffer;
                status = UnlockFile(req->FilePath);

                if (NT_SUCCESS(status)) {
                    Log("[XM] 解锁文件成功 %ws", req->FilePath);
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                status = STATUS_UNSUCCESSFUL;
                Log("[XM] CTL_UNLOCK_FILE exception");
            }
            break;
        }

        case CTL_FORCE_DELETE_FILE:
        {
            __try {
                PFILE_REQ req = (PFILE_REQ)Irp->AssociatedIrp.SystemBuffer;
                status = ForceDeleteFile(req->FilePath);

                if (NT_SUCCESS(status)) {
                    Log("[XM] 粉碎文件成功 %ws", req->FilePath);
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                status = STATUS_UNSUCCESSFUL;
                Log("[XM] CTL_FORCE_DELETE_FILE exception");
            }
            break;
        }

        case CTL_ATTACH_MEM_READ:
        {
            PPROCESS_MEM_REQ memReq = (PPROCESS_MEM_REQ)Irp->AssociatedIrp.SystemBuffer;
            __try {
                KdPrint(("[XM] CTL_ATTACH_MEM_READ ProcessId:%p Address:%p Size:%d\n",
                    memReq->ProcessId, memReq->VirtualAddress, memReq->Size));
                HANDLE processId = memReq->ProcessId;
                PVOID VirtualAddress = memReq->VirtualAddress;
                unsigned Size = memReq->Size;

                status = MemApiRead(processId, VirtualAddress,
                    Irp->AssociatedIrp.SystemBuffer, Size);

                if (NT_SUCCESS(status)) {
                    info = Size;
                    KdPrint(("[XM] CTL_ATTACH_MEM_READ Success\n"));
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                status = STATUS_UNSUCCESSFUL;
                KdPrint(("[XM] CTL_ATTACH_MEM_READ exception\n"));
            }
            break;
        }

        case CTL_ATTACH_MEM_WRITE:
        {
            PPROCESS_MEM_REQ memReq = (PPROCESS_MEM_REQ)Irp->AssociatedIrp.SystemBuffer;
            __try {
                KdPrint(("[XM] CTL_ATTACH_MEM_WRITE ProcessId:%p Address:%p Size:%d\n",
                    memReq->ProcessId, memReq->VirtualAddress, memReq->Size));

                //要写的数据在请求头后
                PVOID writeData = (PUCHAR)Irp->AssociatedIrp.SystemBuffer + sizeof(PROCESS_MEM_REQ);
                status = MemApiWrite(memReq->ProcessId, memReq->VirtualAddress,
                    writeData, memReq->Size);

                if (NT_SUCCESS(status)) {
                    info = sizeof(PROCESS_MEM_REQ);
                    KdPrint(("[XM] CTL_ATTACH_MEM_WRITE Success\n"));
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                status = STATUS_UNSUCCESSFUL;
                KdPrint(("[XM] CTL_ATTACH_MEM_WRITE exception\n"));
            }
            break;
        }


        default:
            Log("[XM] DispatchDeviceControl: 无效控制码 0x%08X", controlCode);
            status = STATUS_INVALID_DEVICE_REQUEST;
            break;
    }

    return CompleteRequest(Irp, info, status);
}

VOID Unload(__in struct _DRIVER_OBJECT* DriverObject)
{
    UNREFERENCED_PARAMETER(DriverObject);
    UNICODE_STRING usSymbolicLinkName;

    // 删除符号链接
    RtlInitUnicodeString(&usSymbolicLinkName, SYMBOL_NAME);
    IoDeleteSymbolicLink(&usSymbolicLinkName);

    // 删除设备对象
    if (DriverObject->DeviceObject != nullptr) {
        IoDeleteDevice(DriverObject->DeviceObject);
    }

    Log("[XM] Unload 卸载驱动 ");
}

NTSTATUS DriverEntry(
    __in struct _DRIVER_OBJECT* DriverObject, __in PUNICODE_STRING RegistryPath)
{
    UNREFERENCED_PARAMETER(RegistryPath);

    // 保存驱动对象
    g_DriverObject = DriverObject;

    Log("[XM] DriverEntry DriverObject:%p RegistryPath:%wZ ",
        DriverObject, RegistryPath);

    //设置卸载函数
    DriverObject->DriverUnload = Unload;

    //设置主要功能回调
    DriverObject->MajorFunction[IRP_MJ_CREATE] = DispatchCreate;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = DispatchClose;
    DriverObject->MajorFunction[IRP_MJ_READ] = DispatchRead;
    DriverObject->MajorFunction[IRP_MJ_WRITE] = DispatchWrite;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchDeviceControl;

    // 创建设备对象
    PDEVICE_OBJECT pDevObj = NULL;
    UNICODE_STRING usDeviceName;
    RtlInitUnicodeString(&usDeviceName, DEVICE_NAME);
    NTSTATUS status = STATUS_UNSUCCESSFUL;

    do {
        status = IoCreateDevice(
            DriverObject,           // 驱动对象
            0,                      // 设备类型
            &usDeviceName,          // 设备名称
            FILE_DEVICE_UNKNOWN,    // 设备类型
            FILE_DEVICE_SECURE_OPEN,// 安全打开
            FALSE,                  // 不共享
            &pDevObj               // 设备对象指针
        );

        if (!NT_SUCCESS(status))
        {
            Log("[XM] Driver Entry IoCreateDevice 失败 %08x ", status);
            break;
        }
        Log("[XM] Driver Entry IoCreateDevice Ok pDevObj:%p ", pDevObj);

        // 设置设备标志
        pDevObj->Flags |= DO_BUFFERED_IO;          // 使用缓冲IO
        pDevObj->Flags &= ~DO_DEVICE_INITIALIZING; // 清除设备初始化标志

        //创建符号链接
        UNICODE_STRING usSymbolicLinkName;
        RtlInitUnicodeString(&usSymbolicLinkName, SYMBOL_NAME);

        status = IoCreateSymbolicLink(&usSymbolicLinkName, &usDeviceName);
        if (!NT_SUCCESS(status))
        {
            Log("[XM] IoCreateSymbolicLink 失败 %08x ", status);
            IoDeleteDevice(pDevObj);
            break;
        }
        Log("[XM] IoCreateSymbolicLink Ok ");

    } while (false);

    return status;
}

