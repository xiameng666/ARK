#include "driver.h"


// ȫ�ֱ�������
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

    // ���Ƶ�ȫ��Pdb��ַ
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
            Log("[XM] DispatchWrite: �յ�VA=0x%p", (PVOID)g_VA);
            return CompleteRequest(Irp, sizeof(ULONG_PTR));
        }
    }
    
    Log("[XM] DispatchWrite: ��Ч���ݳ��� %d", length);
    return CompleteRequest(Irp, 0, STATUS_INVALID_PARAMETER);
}

NTSTATUS DispatchDeviceControl(_In_ struct _DEVICE_OBJECT* DeviceObject, _Inout_ struct _IRP* Irp) {
    UNREFERENCED_PARAMETER(DeviceObject);
    
    PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
    ULONG controlCode = stack->Parameters.DeviceIoControl.IoControlCode;
    NTSTATUS status = STATUS_INVALID_DEVICE_REQUEST;
    ULONG_PTR information = 0;

    switch (controlCode) {
        case CTL_SET_PDB_PATH:  //R3����PDB->��·����R0->R0����PDB��ʼ��ƫ��
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
                    Log("[XM] CTL_ENUM_PROCESS_COUNT: %d ������", processCount);
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
                    Log("[XM] CTL_ENUM_PROCESS: ���� %d ��������Ϣ", processCount);
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
                    Log("[XM] CTL_ENUM_MODULE_COUNT: %d ��ģ��", moduleCount);
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
                    Log("[XM] CTL_ENUM_MODULE: ���� %d ��ģ����Ϣ", moduleCount);
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
                    Log("[XM] CTL_ENUM_MODULE: ���� %d ��SSDT����", ssdtCount);
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                status = STATUS_UNSUCCESSFUL;
                Log("[XM] CTL_ENUM_SSDT exception");
            }
            break;
        }

        //case CTL_ENUM_CALLBACK_COUNT: ����Ҫ�� ������R3����޴󻺳������ɡ�
       
        case CTL_ENUM_CALLBACK:
        {
            __try {
                // R3���ͻص����ͣ�R0���ظ����͵����лص���Ϣ
                PULONG callbackType = (PULONG)Irp->AssociatedIrp.SystemBuffer;
                ULONG callbackCount = 0;
                
                status = EnumCallbacks((PCALLBACK_INFO)Irp->AssociatedIrp.SystemBuffer, 
                                     (CALLBACK_TYPE)*callbackType, 
                                     &callbackCount);
                                     
                if (NT_SUCCESS(status)) {
                    information = callbackCount * sizeof(CALLBACK_INFO);
                    Log("[XM] CTL_ENUM_CALLBACK: ���� %d ���� %d ���ص���Ϣ", *callbackType, callbackCount);
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
                    Log("[XM] CTL_RESTORE_CALLBACK: �ɹ�ɾ���ص�������=%d������=%d", 
                        restoreReq->Type, restoreReq->Index);
                } else {
                    Log("[XM] CTL_RESTORE_CALLBACK: ɾ���ص�ʧ�ܣ�����=%d������=%d", 
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
            Log("[XM] DispatchDeviceControl: ��������� 0x%08X", controlCode);
            status = STATUS_INVALID_DEVICE_REQUEST;
            break;
    }

    return CompleteRequest(Irp, information, status);
}

VOID Unload(__in struct _DRIVER_OBJECT* DriverObject)
{
    UNREFERENCED_PARAMETER(DriverObject);
    UNICODE_STRING usSymbolicLinkName;

    // ɾ����������
    RtlInitUnicodeString(&usSymbolicLinkName, SYMBOL_NAME);
    IoDeleteSymbolicLink(&usSymbolicLinkName);

    // ɾ���豸����
    if (DriverObject->DeviceObject != nullptr) {
        IoDeleteDevice(DriverObject->DeviceObject);
    }

    Log("[XM] Unload ��� ");
}

NTSTATUS DriverEntry(
    __in struct _DRIVER_OBJECT* DriverObject, __in PUNICODE_STRING RegistryPath)
{
    UNREFERENCED_PARAMETER(RegistryPath);

    Log("[XM] DriverEntry DriverObject:%p RegistryPath:%wZ ",
        DriverObject, RegistryPath);

    //����unload
    DriverObject->DriverUnload = Unload;

    //���÷ַ����� �ص�������Ӳ���豸ͨѶ 
    DriverObject->MajorFunction[IRP_MJ_CREATE] = DispatchCreate;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = DispatchClose;
    DriverObject->MajorFunction[IRP_MJ_READ] = DispatchRead;
    DriverObject->MajorFunction[IRP_MJ_WRITE] = DispatchWrite;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchDeviceControl;

    // �����豸����
    PDEVICE_OBJECT pDevObj = NULL;
    UNICODE_STRING usDeviceName;
    RtlInitUnicodeString(&usDeviceName, DEVICE_NAME);
    NTSTATUS status = STATUS_UNSUCCESSFUL;

    do {
        status = IoCreateDevice(
            DriverObject,           // ��������
            0,                      // �豸��չ��С
            &usDeviceName,          // �豸����
            FILE_DEVICE_UNKNOWN,    // �豸����
            FILE_DEVICE_SECURE_OPEN,// �豸����  Ȩ��
            FALSE,                  // ��ռ���� 
            &pDevObj               // ���ص��豸���� 
        );

        if (!NT_SUCCESS(status))
        {
            Log("[XM] Driver Entry IoCreateDevice ErrCode:%08x ", status);
            break;
        }
        Log("[XM] Driver Entry IoCreateDevice Ok pDevObj:%p ", pDevObj);

        // �����豸��־
        pDevObj->Flags |= DO_BUFFERED_IO;          // ʹ�û���IO
        pDevObj->Flags &= ~DO_DEVICE_INITIALIZING; // �����ʼ����־

        //������������  ͨѶ
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

