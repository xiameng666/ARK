#include "DriverBase.h"

extern "C" void _sgdt(void*);
BOOLEAN g_LogOn = TRUE;
ULONG_PTR g_VA = 0;
PDRIVER_OBJECT g_DriverObject = NULL;  // 保存当前驱动对象

wchar_t g_PdbDownloadPath[512] = L"C:\\Symbols";

NTSTATUS SetPdbPath(PWCHAR InputPath)
{
    if (!InputPath) {
        return STATUS_INVALID_PARAMETER;
    }

    SIZE_T pathLength = wcslen(InputPath);

    RtlZeroMemory(g_PdbDownloadPath, sizeof(g_PdbDownloadPath));
    RtlCopyMemory(g_PdbDownloadPath, InputPath, pathLength * sizeof(WCHAR));

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
                SetPdbPath(pdbPathReq->DownloadPath);
                status = STATUS_SUCCESS;
                Log("[XM] CTL_SET_PDB_PATH: %ws", g_PdbDownloadPath);
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
        
        case CTL_ENUM_PROCESS_COUNT:
        {
            __try {
                //KdBreakPoint();
                ULONG processCount = 0;
                status = EnumProcessFromLinksEx(NULL, TRUE, &processCount);
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
        
        case CTL_ENUM_PROCESS:
        {
            __try {
                ULONG processCount = 0;
                status = EnumProcessFromLinksEx((PPROCESS_INFO)Irp->AssociatedIrp.SystemBuffer, FALSE, &processCount);
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

        case CTL_ENUM_SSDT:
        {
            __try {
                ULONG ssdtCount = 0;
                status = EnumSSDT((PSSDT_INFO)Irp->AssociatedIrp.SystemBuffer,&ssdtCount);
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
                Log("[ARK] Successfully force terminated process %d", (ULONG)PID);
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

