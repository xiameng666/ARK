#include "file.h"

extern "C" NTSTATUS ZwQuerySystemInformation(
    __in SYSTEM_INFORMATION_CLASS SystemInformationClass,
    __out_bcount_opt(SystemInformationLength) PVOID SystemInformation,
    __in ULONG SystemInformationLength,
    __out_opt PULONG ReturnLength
);

// EX版本的句柄结构体定义
typedef struct _SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX {
    PVOID Object;
    ULONG_PTR UniqueProcessId;
    ULONG_PTR HandleValue;
    ULONG GrantedAccess;
    USHORT CreatorBackTraceIndex;
    USHORT ObjectTypeIndex;
    ULONG HandleAttributes;
    ULONG Reserved;
} SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX, *PSYSTEM_HANDLE_TABLE_ENTRY_INFO_EX;

typedef struct _SYSTEM_HANDLE_INFORMATION_EX {
    ULONG_PTR NumberOfHandles;
    ULONG_PTR Reserved;
    SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX Handles[1];
} SYSTEM_HANDLE_INFORMATION_EX, *PSYSTEM_HANDLE_INFORMATION_EX;

typedef NTSTATUS (NTAPI *RtlDosPathNameToNtPathName_U)(
    IN PWSTR DosFileName,
    OUT PUNICODE_STRING NtFileName,
    OUT PWSTR *FilePart OPTIONAL,
    OUT PULONG RelativeName OPTIONAL
);

// UnlockFile / ForceDeleteFile
NTSTATUS UnlockFile(WCHAR* filePath) {

    //遍历所有进程句柄表
    NTSTATUS Status;
    PSYSTEM_HANDLE_INFORMATION_EX      HandlesEx;
    PSYSTEM_HANDLE_TABLE_ENTRY_INFO_EX HandleInfoEx;
    POBJECT_NAME_INFORMATION  ObjectNameInfo;
    PVOID Buffer;
    ULONG BufferSize = 4096;
    ULONG ReturnLength;
    ULONG_PTR i;
    UNICODE_STRING ustrName;

    RtlInitUnicodeString(&ustrName, filePath);
    Log("[XM] UnlockFile ustr: %wZ", &ustrName);

    ObjectNameInfo = (POBJECT_NAME_INFORMATION)ExAllocatePoolWithTag(NonPagedPool, 4096, 'ULFL');
    if (!ObjectNameInfo) {
        return STATUS_NO_MEMORY;
    }

retry:
    Buffer = ExAllocatePoolWithTag(NonPagedPool, BufferSize, 'ULFL');
    if (!Buffer) {
        ExFreePool(ObjectNameInfo);
        return STATUS_NO_MEMORY;
    }
    Status = ZwQuerySystemInformation(SystemExtendedHandleInformation,
        Buffer,
        BufferSize,
        &ReturnLength
    );

    if (Status == STATUS_INFO_LENGTH_MISMATCH) {
        ExFreePool(Buffer);
        BufferSize = ReturnLength;
        goto retry;
    }

    if (NT_SUCCESS(Status)) {

        HandlesEx = (PSYSTEM_HANDLE_INFORMATION_EX)Buffer;

        Log("[XM] 开始遍历句柄，总数: %llu", HandlesEx->NumberOfHandles);

        for (i = 0; i < HandlesEx->NumberOfHandles; i++) {
            HandleInfoEx = &(HandlesEx->Handles[i]);
            Status = ObReferenceObjectByPointer(HandleInfoEx->Object, 0,
                *IoFileObjectType, KernelMode);//*IoFileObjectType

            if (NT_SUCCESS(Status)) {
                Status = ObQueryNameString(HandleInfoEx->Object, ObjectNameInfo, 4096, &ReturnLength);

                if (NT_SUCCESS(Status)) {
                                      
                    if (RtlCompareUnicodeString(&ObjectNameInfo->Name, &ustrName, TRUE) == 0) {
                        Log("[XM] 找到匹配句柄: PID=%llu, Handle=%llu, Path=%wZ",
                            HandleInfoEx->UniqueProcessId,
                            HandleInfoEx->HandleValue,
                            &ObjectNameInfo->Name);

                        //切换进程
                        PEPROCESS Process = NULL;
                        Status = PsLookupProcessByProcessId((HANDLE)HandleInfoEx->UniqueProcessId, &Process);

                        if (NT_SUCCESS(Status)) {
                            KAPC_STATE ApcState;
                            KeStackAttachProcess(Process, &ApcState);
                            Status = ZwClose((HANDLE)HandleInfoEx->HandleValue);

                            Log("[XM] unlock UniqueProcessId:%llu HandleValue:%llu Name:%wZ",
                                HandleInfoEx->UniqueProcessId,
                                HandleInfoEx->HandleValue,
                                &ObjectNameInfo->Name);

                            KeUnstackDetachProcess(&ApcState);
                            ObDereferenceObject(Process);
                        }
                    }
                }
                ObDereferenceObject(HandleInfoEx->Object);
            }
        }

    }

    ExFreePool(ObjectNameInfo);
    ExFreePool(Buffer);
    return Status;
}

NTSTATUS ForceDeleteFile(WCHAR* filePath) {
    NTSTATUS Status;
    UNICODE_STRING ustrName;
    RtlInitUnicodeString(&ustrName, filePath);
    Log("[XM] ForceDeleteFile 路径: %wZ", &ustrName);

    Status = UnlockFile(filePath);

    OBJECT_ATTRIBUTES objAttr;
    InitializeObjectAttributes(&objAttr, &ustrName, OBJ_CASE_INSENSITIVE, NULL, NULL);

    Status = ZwDeleteFile(&objAttr);
    if (NT_SUCCESS(Status)) {
        Log("[XM] 粉碎文件成功: %ws", filePath);
    }
    else {
        Log("[XM] 粉碎文件失败: %ws, STATUS: 0x%08X", filePath, Status);
    }

    return Status;
}

