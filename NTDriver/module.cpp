#include "module.h"

extern "C"
NTSTATUS NTAPI ZwQuerySystemInformation(
    __in SYSTEM_INFORMATION_CLASS SystemInformationClass,
    __out_bcount_opt(SystemInformationLength) PVOID SystemInformation,
    __in ULONG SystemInformationLength,
    __out_opt PULONG ReturnLength
);

NTSTATUS EnumModuleEx(PMODULE_INFO ModuleBuffer, bool CountOnly, PULONG ModuleCount)
{
    NTSTATUS Status;
    PRTL_PROCESS_MODULES Modules;
    PRTL_PROCESS_MODULE_INFORMATION ModuleInfo;
    PVOID Buffer;
    ULONG BufferSize = 4096;
    ULONG ReturnLength;
    ULONG i;
    ULONG Count = 0;

    Log("[XM] EnumModuleEx: CountOnly=%d ", CountOnly);

retry:
    Buffer = ExAllocatePoolWithTag(NonPagedPool, BufferSize, 'MDLE');
    if (!Buffer) {
        return STATUS_NO_MEMORY;
    }

    Status = ZwQuerySystemInformation(SystemModuleInformation,
        Buffer,
        BufferSize,
        &ReturnLength
    );

    if (Status == STATUS_INFO_LENGTH_MISMATCH) {
        ExFreePool(Buffer);
        BufferSize = ReturnLength;
        goto retry;
    }

    if (!NT_SUCCESS(Status)) {
        ExFreePool(Buffer);
        return Status;
    }

    Modules = (PRTL_PROCESS_MODULES)Buffer;
    Count = Modules->NumberOfModules;

    if (CountOnly) {
        *ModuleCount = Count;
        ExFreePool(Buffer);
        return STATUS_SUCCESS;
    }

    // 填充模块信息
    for (i = 0, ModuleInfo = &(Modules->Modules[0]);
        i < Modules->NumberOfModules && ModuleBuffer;
        i++, ModuleInfo++) {

        __try {
            // 清空结构体
            RtlZeroMemory(&ModuleBuffer[i], sizeof(MODULE_INFO));

            // 提取模块名称（从完整路径中提取文件名）
            CHAR* fileName = (CHAR*)ModuleInfo->FullPathName + ModuleInfo->OffsetToFileName;
            RtlCopyMemory(ModuleBuffer[i].Name, fileName,
                min(strlen(fileName), sizeof(ModuleBuffer[i].Name) - 1));

            // 完整路径
            RtlCopyMemory(ModuleBuffer[i].FullPath, ModuleInfo->FullPathName,
                min(strlen((CHAR*)ModuleInfo->FullPathName), sizeof(ModuleBuffer[i].FullPath) - 1));

            // 基地址和大小
            ModuleBuffer[i].ImageBase = ModuleInfo->ImageBase;
            ModuleBuffer[i].ImageSize = ModuleInfo->ImageSize;
            ModuleBuffer[i].LoadOrderIndex = ModuleInfo->LoadOrderIndex;
            ModuleBuffer[i].LoadCount = ModuleInfo->LoadCount;


            Log("[XM] Module[%d]: Name=%s, Base=%p, Size=0x%X ",
                i, ModuleBuffer[i].Name, ModuleBuffer[i].ImageBase, ModuleBuffer[i].ImageSize);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            Log("[XM] EnumProcessModuleEx: Exception processing module %d ", i);
            continue;
        }
    }

    *ModuleCount = Count;
    ExFreePool(Buffer);
    return STATUS_SUCCESS;
}
