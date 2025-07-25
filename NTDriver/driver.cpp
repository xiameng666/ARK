#include "driver.h"


// 全局变量定义
extern "C" void _sgdt(void*);
BOOLEAN g_LogOn = TRUE;
ULONG_PTR g_VA = 0;

wchar_t g_PdbDownloadPath[512] = L"C:\\Symbols";

NTSTATUS SetPdbPath(PWCHAR InputPath)
{
    if (!InputPath) {
        return STATUS_INVALID_PARAMETER;
    }

    SIZE_T pathLength = wcslen(InputPath);

    // 复制到全局Pdb地址
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
            Log("[XM] DispatchWrite: 收到VA=0x%p", (PVOID)g_VA);
            return CompleteRequest(Irp, sizeof(ULONG_PTR));
        }
    }
    
    Log("[XM] DispatchWrite: 无效数据长度 %d", length);
    return CompleteRequest(Irp, 0, STATUS_INVALID_PARAMETER);
}

NTSTATUS DispatchDeviceControl(_In_ struct _DEVICE_OBJECT* DeviceObject, _Inout_ struct _IRP* Irp) {
    UNREFERENCED_PARAMETER(DeviceObject);
    
    PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
    ULONG controlCode = stack->Parameters.DeviceIoControl.IoControlCode;
    NTSTATUS status = STATUS_INVALID_DEVICE_REQUEST;
    ULONG_PTR information = 0;

    switch (controlCode) {
        case CTL_SET_PDB_PATH:  //R3下载PDB->发路径给R0->R0解析PDB初始化偏移
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
                information = gdtSize;
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
                status = EnumProcessEx(NULL, TRUE, &processCount);
                if (NT_SUCCESS(status)) {
                    *(PULONG)Irp->AssociatedIrp.SystemBuffer = processCount;
                    information = sizeof(ULONG);
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
                status = EnumProcessEx((PPROCESS_INFO)Irp->AssociatedIrp.SystemBuffer, FALSE, &processCount);
                if (NT_SUCCESS(status)) {
                    information = processCount * sizeof(PROCESS_INFO);
                    Log("[XM] CTL_ENUM_PROCESS: 返回 %d 个进程信息", processCount);
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
                    information = sizeof(ULONG);
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
                    information = moduleCount * sizeof(MODULE_INFO);
                    Log("[XM] CTL_ENUM_MODULE: 返回 %d 个模块信息", moduleCount);
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
                    information = ssdtCount * sizeof(SSDT_INFO);
                    Log("[XM] CTL_ENUM_MODULE: 返回 %d 个SSDT函数", ssdtCount);
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                status = STATUS_UNSUCCESSFUL;
                Log("[XM] CTL_ENUM_SSDT exception");
            }
            break;
        }

        //case CTL_ENUM_CALLBACK_COUNT: 不需要了 我们在R3申请巨大缓冲区即可。
       
        case CTL_ENUM_CALLBACK:
        {
            __try {
                // R3发送回调类型，R0返回该类型的所有回调信息
                PULONG callbackType = (PULONG)Irp->AssociatedIrp.SystemBuffer;
                ULONG callbackCount = 0;
                
                status = EnumCallbacks((PCALLBACK_INFO)Irp->AssociatedIrp.SystemBuffer, 
                                     (CALLBACK_TYPE)*callbackType, 
                                     &callbackCount);
                                     
                if (NT_SUCCESS(status)) {
                    information = callbackCount * sizeof(CALLBACK_INFO);
                    Log("[XM] CTL_ENUM_CALLBACK: 类型 %d 返回 %d 个回调信息", *callbackType, callbackCount);
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                status = STATUS_UNSUCCESSFUL;
                Log("[XM] CTL_ENUM_CALLBACK exception");
            }
            break;
        }
        /*
        case CTL_DELETE_CALLBACK:
        {
            __try {
                PRESTORE_CALLBACK_REQUEST restoreReq = (PRESTORE_CALLBACK_REQUEST)Irp->AssociatedIrp.SystemBuffer;
                
                status = DeleteCallback(restoreReq->Type, restoreReq->Index);
                
                if (NT_SUCCESS(status)) {
                    Log("[XM] CTL_RESTORE_CALLBACK: 成功删除回调，类型=%d，索引=%d", 
                        restoreReq->Type, restoreReq->Index);
                } else {
                    Log("[XM] CTL_RESTORE_CALLBACK: 删除回调失败，类型=%d，索引=%d", 
                        restoreReq->Type, restoreReq->Index);
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                status = STATUS_UNSUCCESSFUL;
                Log("[XM] CTL_RESTORE_CALLBACK exception");
            }
            break;
        }
        */

        default:
            Log("[XM] DispatchDeviceControl: 错误控制码 0x%08X", controlCode);
            status = STATUS_INVALID_DEVICE_REQUEST;
            break;
    }

    return CompleteRequest(Irp, information, status);
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

    Log("[XM] Unload 完成 ");
}

NTSTATUS DriverEntry(
    __in struct _DRIVER_OBJECT* DriverObject, __in PUNICODE_STRING RegistryPath)
{
    UNREFERENCED_PARAMETER(RegistryPath);

    Log("[XM] DriverEntry DriverObject:%p RegistryPath:%wZ ",
        DriverObject, RegistryPath);

    //设置unload
    DriverObject->DriverUnload = Unload;

    //设置分发例程 回调函数跟硬件设备通讯 
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
            0,                      // 设备扩展大小
            &usDeviceName,          // 设备名称
            FILE_DEVICE_UNKNOWN,    // 设备类型
            FILE_DEVICE_SECURE_OPEN,// 设备特性  权限
            FALSE,                  // 独占访问 
            &pDevObj               // 返回的设备对象 
        );

        if (!NT_SUCCESS(status))
        {
            Log("[XM] Driver Entry IoCreateDevice ErrCode:%08x ", status);
            break;
        }
        Log("[XM] Driver Entry IoCreateDevice Ok pDevObj:%p ", pDevObj);

        // 设置设备标志
        pDevObj->Flags |= DO_BUFFERED_IO;          // 使用缓冲IO
        pDevObj->Flags &= ~DO_DEVICE_INITIALIZING; // 清除初始化标志

        //创建符号链接  通讯
        UNICODE_STRING usSymbolicLinkName;
        RtlInitUnicodeString(&usSymbolicLinkName, SYMBOL_NAME);

        status = IoCreateSymbolicLink(&usSymbolicLinkName, &usDeviceName);
        if (!NT_SUCCESS(status))
        {
            Log("[XM] IoCreateSymbolicLink ErrCode:%08x ", status);
            IoDeleteDevice(pDevObj);
            break;
        }
        Log("[XM] IoCreateSymbolicLink Ok ");

    } while (false);

    return status;
}

