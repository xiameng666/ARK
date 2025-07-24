#include "driver.h"


// 全局变量定义
extern "C" void _sgdt(void*);
BOOLEAN g_LogOn = TRUE;
ULONG_PTR g_VA = 0;

// 全局PDB下载路径变量
wchar_t g_PdbDownloadPath[256] = { 0 };
BOOLEAN g_HasCustomPdbPath = FALSE;

// PDB路径管理函数实现
void InitializePdbPath() {
    wcscpy_s(g_PdbDownloadPath, L"C:\\Windows\\Temp\\");
    g_HasCustomPdbPath = FALSE;
    Log("[XM] InitializePdbPath: 初始化默认PDB路径为 %ws", g_PdbDownloadPath);
}

void SetGlobalPdbDownloadPath(const wchar_t* path) {
    if (!path) {
        Log("[XM] SetGlobalPdbDownloadPath: 路径为空");
        return;
    }
    
    wcscpy_s(g_PdbDownloadPath, path);
    
    // 确保路径以反斜杠结尾
    size_t len = wcslen(g_PdbDownloadPath);
    if (len > 0 && g_PdbDownloadPath[len - 1] != L'\\') {
        if (len < 255) {
            g_PdbDownloadPath[len] = L'\\';
            g_PdbDownloadPath[len + 1] = L'\0';
        }
    }
    
    g_HasCustomPdbPath = TRUE;
    Log("[XM] SetGlobalPdbDownloadPath: 设置PDB路径为 %ws", g_PdbDownloadPath);
}

void SetGlobalPdbDownloadPath(const UNICODE_STRING* path) {
    if (!path || !path->Buffer || path->Length == 0) {
        Log("[XM] SetGlobalPdbDownloadPath: UNICODE_STRING路径无效");
        return;
    }
    
    // 复制路径（确保不超过缓冲区大小）
    size_t copyLen = min(path->Length / sizeof(WCHAR), 255);
    wcsncpy_s(g_PdbDownloadPath, path->Buffer, copyLen);
    
    // 确保路径以反斜杠结尾
    size_t len = wcslen(g_PdbDownloadPath);
    if (len > 0 && g_PdbDownloadPath[len - 1] != L'\\') {
        if (len < 255) {
            g_PdbDownloadPath[len] = L'\\';
            g_PdbDownloadPath[len + 1] = L'\0';
        }
    }
    
    g_HasCustomPdbPath = TRUE;
    Log("[XM] SetGlobalPdbDownloadPath: 从UNICODE_STRING设置PDB路径为 %ws", g_PdbDownloadPath);
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
        case CTL_SET_PDB_PATH:
        {
            __try {
                PPDB_PATH_REQUEST pdbPathReq = (PPDB_PATH_REQUEST)Irp->AssociatedIrp.SystemBuffer;
                if (pdbPathReq && stack->Parameters.DeviceIoControl.InputBufferLength >= sizeof(PDB_PATH_REQUEST)) {
                    SetGlobalPdbDownloadPath(pdbPathReq->DownloadPath);
                    status = STATUS_SUCCESS;
                    Log("[XM] CTL_SET_PDB_PATH: 成功设置PDB路径为 %ws", pdbPathReq->DownloadPath);
                } else {
                    status = STATUS_INVALID_PARAMETER;
                    Log("[XM] CTL_SET_PDB_PATH: 无效参数");
                }
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
                
                // 设置CPU亲和性
                KAFFINITY oldAffinity = KeSetSystemAffinityThreadEx(1ULL << cpuIndex);
                
                _sgdt(&gdtr);
                
                ULONG gdtSize = gdtr.Limit + 1;
                Log("[XM] CpuIndex: %d GdtBase: %p GdtLimit: %08x, Size: %d",
                    cpuIndex, (PVOID)gdtr.Base, gdtr.Limit, gdtSize);
                
                if (MmIsAddressValid((PVOID)gdtr.Base)) {
                    RtlCopyMemory(Irp->AssociatedIrp.SystemBuffer, (PVOID)gdtr.Base, gdtSize);
                    information = gdtSize;
                    status = STATUS_SUCCESS;
                } else {
                    Log("[XM] Invalid GDT Base address: %p", (PVOID)gdtr.Base);
                    status = STATUS_INVALID_ADDRESS;
                }
                
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
        
        default:
            Log("[XM] DispatchDeviceControl: 未知控制码 0x%08X", controlCode);
            status = STATUS_INVALID_DEVICE_REQUEST;
            break;
    }

    return CompleteRequest(Irp, information, status);
}

VOID Unload(__in struct _DRIVER_OBJECT* DriverObject) {
    Log("[XM] 驱动卸载");
    
    UNICODE_STRING symbolicLink = RTL_CONSTANT_STRING(L"\\??\\ArkDevice");
    IoDeleteSymbolicLink(&symbolicLink);
    
    if (DriverObject->DeviceObject) {
        IoDeleteDevice(DriverObject->DeviceObject);
    }
}

extern "C" NTSTATUS DriverEntry(
    __in struct _DRIVER_OBJECT* DriverObject,
    __in PUNICODE_STRING RegistryPath) {
    
    UNREFERENCED_PARAMETER(RegistryPath);
    
    Log("[XM] DriverEntry: 驱动加载开始");
    
    // 初始化PDB路径
    InitializePdbPath();
    
    // 创建设备对象
    UNICODE_STRING deviceName = RTL_CONSTANT_STRING(L"\\Device\\ArkDevice");
    UNICODE_STRING symbolicLink = RTL_CONSTANT_STRING(L"\\??\\ArkDevice");
    
    PDEVICE_OBJECT deviceObject = nullptr;
    NTSTATUS status = IoCreateDevice(
        DriverObject,
        sizeof(MY_DEV_EXT),
        &deviceName,
        FILE_DEVICE_UNKNOWN,
        FILE_DEVICE_SECURE_OPEN,
        FALSE,
        &deviceObject
    );
    
    if (!NT_SUCCESS(status)) {
        Log("[XM] DriverEntry: 创建设备对象失败 0x%08X", status);
        return status;
    }
    
    status = IoCreateSymbolicLink(&symbolicLink, &deviceName);
    if (!NT_SUCCESS(status)) {
        Log("[XM] DriverEntry: 创建符号链接失败 0x%08X", status);
        IoDeleteDevice(deviceObject);
        return status;
    }
    
    // 设置派发函数
    DriverObject->MajorFunction[IRP_MJ_CREATE] = DispatchCreate;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = DispatchClose;
    DriverObject->MajorFunction[IRP_MJ_READ] = DispatchRead;
    DriverObject->MajorFunction[IRP_MJ_WRITE] = DispatchWrite;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchDeviceControl;
    DriverObject->DriverUnload = Unload;
    
    // 设置设备标志
    deviceObject->Flags |= DO_BUFFERED_IO;
    deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
    
    Log("[XM] DriverEntry: 驱动加载成功");
    return STATUS_SUCCESS;
}

