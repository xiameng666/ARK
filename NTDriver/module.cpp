#include "module.h"

// 全局遍历模块数据缓存
PRTL_PROCESS_MODULES g_ModuleBuffer = nullptr; //cache

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


NTSTATUS EnumModule()
{
    NTSTATUS Status;
    PVOID Buffer;
    ULONG BufferSize = 4096;
    ULONG ReturnLength;

    Log("[XM] EnumModule: Caching module information globally");

retry:
    Buffer = ExAllocatePoolWithTag(NonPagedPool, BufferSize, 'EMUD');
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

 
    if (g_ModuleBuffer) {
        ExFreePool(g_ModuleBuffer);
    }

    // Cache the buffer globally
    g_ModuleBuffer = (PRTL_PROCESS_MODULES)Buffer;  

    Log("[XM] EnumModule: Successfully cached %d modules", g_ModuleBuffer->NumberOfModules);

    return STATUS_SUCCESS;
}

//拆分FindModuleByAddress 改为 EnumModule（遍历模块写入缓存） +FindModuleByAddress（从缓存拿数据）
NTSTATUS FindModuleByAddress(PVOID Address, PCHAR ModulePath, PVOID* ImageBase, PULONG ImageSize)
{
    PRTL_PROCESS_MODULE_INFORMATION ModuleInfo;
    ULONG i;
    ULONG_PTR SearchAddress = (ULONG_PTR)Address;
    ULONG_PTR ModuleStart, ModuleEnd;

    if (!g_ModuleBuffer) {
        Log("[XM] FindModuleByAddress: !g_ModuleBuffer");
        return STATUS_UNSUCCESSFUL;
    }

    for (i = 0, ModuleInfo = &(g_ModuleBuffer->Modules[0]);
        i < g_ModuleBuffer->NumberOfModules;
        i++, ModuleInfo++) {

        ModuleStart = (ULONG_PTR)ModuleInfo->ImageBase;
        ModuleEnd = ModuleStart + ModuleInfo->ImageSize;
        if (!MmIsAddressValid((PVOID)ModuleStart) || !MmIsAddressValid((PVOID)ModuleStart)) {
            continue;
        }

        // 如果地址在模块范围内
        if (SearchAddress >= ModuleStart && SearchAddress < ModuleEnd) {
            __try {
                
                if (ModulePath) {
                    size_t srcLength = strlen((CHAR*)ModuleInfo->FullPathName);
                    size_t copyLength = min(srcLength, 255);
                    
                    RtlZeroMemory(ModulePath, 256); 
                    RtlCopyMemory(ModulePath, ModuleInfo->FullPathName, copyLength);
                    ModulePath[copyLength] = '\0'; 
                    
                }

                if (ImageBase) {
                    *ImageBase = ModuleInfo->ImageBase;
                }

                if (ImageSize) {
                    *ImageSize = ModuleInfo->ImageSize;
                }

                Log("[XM] FindModuleByAddress: Found module %s for address %p (Base=%p, Size=0x%X)",
                    ModuleInfo->FullPathName, Address, ModuleInfo->ImageBase, ModuleInfo->ImageSize);

                return STATUS_SUCCESS;
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                Log("[XM] FindModuleByAddress: Exception ");
                return STATUS_ACCESS_VIOLATION;
            }
        }
    }

    Log("[XM] FindModuleByAddress: No module found for address %p", Address);
    return STATUS_NOT_FOUND;
}
