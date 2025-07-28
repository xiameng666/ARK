#include "driver.h"

#define MAX_OBJECT_STORE 500                           // 最多存储500个驱动对象
DRIVER_OBJECT_INFO g_DriverObjects[MAX_OBJECT_STORE]; 
ULONG g_DriverObjectCount = 0;                         // 枚举得到的驱动数量
extern PDRIVER_OBJECT g_DriverObject ;

NTSTATUS CheckDrvMJHooked(PDISPATCH_HOOK_INFO HookBuffer, PULONG HookCount){
    if (!HookBuffer || !HookCount) {
        return STATUS_INVALID_PARAMETER;
    }

    *HookCount = 0;

    // 遍历存储的驱动对象
    for (ULONG i = 0; i < g_DriverObjectCount; i++) {
        PDRIVER_OBJECT DriverObj = g_DriverObjects[i].DriverObject;

        if (!DriverObj || !MmIsAddressValid(DriverObj)) {
            continue;
        }

        __try {
            PVOID driverStart = DriverObj->DriverStart;
            ULONG driverSize = DriverObj->DriverSize;

            // 检查所有MajorFunction
            for (ULONG j = 0; j <= IRP_MJ_MAXIMUM_FUNCTION && j < 28; j++) {
                PVOID majorFunc = DriverObj->MajorFunction[j];

                BOOLEAN isHooked = FALSE;

                // 检查函数地址是否在驱动模块范围内
                if ((ULONG_PTR)majorFunc < (ULONG_PTR)driverStart ||
                    (ULONG_PTR)majorFunc >= (ULONG_PTR)driverStart + driverSize) {
                    isHooked = TRUE;
                }

                // 填充Hook结构体
                PDISPATCH_HOOK_INFO pHookInfo = &HookBuffer[*HookCount];
                RtlZeroMemory(pHookInfo, sizeof(DISPATCH_HOOK_INFO));

                pHookInfo->MajorFunctionCode = j;
                RtlStringCbCopyA(pHookInfo->FunctionName, sizeof(pHookInfo->FunctionName), majorFunctionNames[j]);

                // 复制驱动名称 (Unicode TO ANSI)
                if (DriverObj->DriverName.Buffer) {
                    ANSI_STRING ansiString;
                    UNICODE_STRING unicodeString = DriverObj->DriverName;
                    RtlUnicodeStringToAnsiString(&ansiString, &unicodeString, TRUE);
                    if (ansiString.Buffer) {
                        RtlStringCbCopyA(pHookInfo->DriverName, sizeof(pHookInfo->DriverName), ansiString.Buffer);
                        RtlFreeAnsiString(&ansiString);
                    }
                }

                pHookInfo->CurrentAddress = majorFunc;
                pHookInfo->IsHooked = isHooked;

                // 查找Hook函数所在的模块
                CHAR hookModulePath[256] = { 0 };
                PVOID hookImageBase = NULL;
                ULONG hookImageSize = 0;

                NTSTATUS findStatus = FindModuleByAddress(majorFunc, hookModulePath, &hookImageBase, &hookImageSize);
                if (NT_SUCCESS(findStatus)) {
                    RtlStringCbCopyA(pHookInfo->CurrentModule, sizeof(pHookInfo->CurrentModule), hookModulePath);
                }
                else {
                    RtlStringCbCopyA(pHookInfo->CurrentModule, sizeof(pHookInfo->CurrentModule), "Unknown");
                }

                (*HookCount)++;

                if (*HookCount >= 1000) {
                    return STATUS_SUCCESS;
                }
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            Log("[XM] CheckDrvMjHooked Exception for driver %d", i);
            continue;
        }
    }

    return STATUS_SUCCESS;
}

void EnumDriverObject() {
    INIT_PDB;
    //遍历驱动对象链表
    PUCHAR DriverObjectByte = (PUCHAR)g_DriverObject;

    ULONG_PTR ObjectHeaderBodyOffset = ntos.GetOffset("_OBJECT_HEADER", "Body");
    if (ObjectHeaderBodyOffset == 0) {
        Log("[XM] Failed to get _OBJECT_HEADER Body offset");
        return;
    }

    // _OBJECT_HEADER_NAME_INFO的大小 暂时硬编码
    ULONG ObjectHeaderNameInfoSize = 0x20;  

    Log("[XM] Body offset: 0x%x, NameInfo size: 0x%x", ObjectHeaderBodyOffset, ObjectHeaderNameInfoSize);

    // OBJECT_HEADER地址
    PUCHAR ObjectHeader = DriverObjectByte - ObjectHeaderBodyOffset;

    // OBJECT_HEADER_NAME_INFO地址 (在OBJECT_HEADER上方)
    PUCHAR PObjHeaderNameInfoPtr = ObjectHeader - ObjectHeaderNameInfoSize;

    __try {
        if (!MmIsAddressValid(PObjHeaderNameInfoPtr)) {
            Log("[XM] Invalid OBJECT_HEADER_NAME_INFO address: %p", PObjHeaderNameInfoPtr);
            return;
        }

        ULONG_PTR DirectoryOffset = ntos.GetOffset("_OBJECT_HEADER_NAME_INFO", "Directory");  //=0
        PVOID* PDirectoryPtr = (PVOID*)(PObjHeaderNameInfoPtr + DirectoryOffset);
        PVOID PDirectory = *PDirectoryPtr;

        if (!PDirectory || !MmIsAddressValid(PDirectory)) {
            Log("[XM] Invalid Directory pointer: %p", PDirectory);
            return;
        }

        Log("[XM] Enumerating Driver Directory at %p", PDirectory);

        ULONG_PTR HashBucketsOffset = ntos.GetOffset("_OBJECT_DIRECTORY", "HashBuckets");//=0
        ULONG_PTR ObjectOffset = ntos.GetOffset("_OBJECT_DIRECTORY_ENTRY", "Object");//=0x08
        ULONG_PTR ChainLinkOffset = ntos.GetOffset("_OBJECT_DIRECTORY_ENTRY", "ChainLink");//=0

        // 遍历37个哈希桶
        PVOID* HashBuckets = (PVOID*)((PUCHAR)PDirectory + HashBucketsOffset);

        for (int i = 0; i < 37; i++) {

            PUCHAR PDirectoryEntry = (PUCHAR)HashBuckets[i];
            if (PDirectoryEntry == NULL) {
                continue;
            }

            PUCHAR PSubDirectoryEntry = PDirectoryEntry;
            while (PSubDirectoryEntry != NULL) {

                PVOID* ObjectPtr = (PVOID*)(PSubDirectoryEntry + ObjectOffset);
                PDRIVER_OBJECT TargetDrvObj = (PDRIVER_OBJECT)*ObjectPtr;

                if (TargetDrvObj->Type == IO_TYPE_DRIVER) {
                    Log("[XM] DrvObj: %p  DrvName: %wZ", TargetDrvObj, &TargetDrvObj->DriverName);

                    // 存储驱动对象信息到全局数据结构
                    if (g_DriverObjectCount < MAX_OBJECT_STORE) {
                        g_DriverObjects[g_DriverObjectCount].DriverObject = TargetDrvObj;

                        // 复制驱动名称
                        if (TargetDrvObj->DriverName.Buffer && TargetDrvObj->DriverName.Length > 0) {
                            ULONG copyLength = min(TargetDrvObj->DriverName.Length, sizeof(g_DriverObjects[g_DriverObjectCount].DriverName) - sizeof(WCHAR));
                            RtlCopyMemory(g_DriverObjects[g_DriverObjectCount].DriverName,
                                TargetDrvObj->DriverName.Buffer, copyLength);
                            g_DriverObjects[g_DriverObjectCount].DriverName[copyLength / sizeof(WCHAR)] = L'\0';
                        }

                        // 存储驱动基地址和大小
                        g_DriverObjects[g_DriverObjectCount].DriverStart = TargetDrvObj->DriverStart;
                        g_DriverObjects[g_DriverObjectCount].DriverSize = TargetDrvObj->DriverSize;

                        g_DriverObjectCount++;
                    }                   
                }

                // 获取下一个ChainLink
                PVOID* ChainLinkPtr = (PVOID*)(PSubDirectoryEntry + ChainLinkOffset);
                PSubDirectoryEntry = (PUCHAR)*ChainLinkPtr;
            }
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        Log("[XM] Exception in EnumDriverObject");
    }
}

//NTSTATUS MajorFunctionisHooked(PDISPATCH_HOOK_INFO HookBuffer, PULONG HookCount)
NTSTATUS MajorFunctionisHooked()
{
    INIT_PDB;
    // 获取PsLoadedModuleList 
    ULONG_PTR PsLoadedModuleList = ntos.GetPointer("PsLoadedModuleList");
    if (!PsLoadedModuleList)
    {
        Log("[XM] MajorFunctionisHooked PsLoadedModuleList is NULL");
        return STATUS_UNSUCCESSFUL;
    }

    ULONG_PTR IoDriverObjectList = ntos.GetPointer("IoDriverObjectList");
    Log("[XM] IoDriverObjectList: %p", IoDriverObjectList);
    // *HookCount = 0;
    ULONG index = 0;

    /*_KLDR_DATA_TABLE_ENTRY
        + 0x000 InLoadOrderLinks : _LIST_ENTRY
        + 0x010 ExceptionTable : Ptr64 Void
        + 0x018 ExceptionTableSize : Uint4B
        + 0x020 GpValue : Ptr64 Void
        + 0x028 NonPagedDebugInfo : Ptr64 _NON_PAGED_DEBUG_INFO
        + 0x030 DllBase : Ptr64 Void
        + 0x038 EntryPoint : Ptr64 Void
        + 0x040 SizeOfImage : Uint4B
        + 0x048 FullDllName : _UNICODE_STRING
        + 0x058 BaseDllName : _UNICODE_STRING
        + 0x068 Flags : Uint4B
        + 0x06c LoadCount : Uint2B
        + 0x06e u1 : <anonymous - tag>
        +0x070 SectionPointer : Ptr64 Void
        + 0x078 CheckSum : Uint4B
        + 0x07c CoverageSectionSize : Uint4B
        + 0x080 CoverageSection : Ptr64 Void
        + 0x088 LoadedImports : Ptr64 Void
        + 0x090 Spare : Ptr64 Void
        + 0x098 SizeOfImageNotRounded : Uint4B
        + 0x09c TimeDateStamp : Uint4B*/

    PLIST_ENTRY moduleList = (PLIST_ENTRY)PsLoadedModuleList;
    for (PLIST_ENTRY entry = moduleList->Flink; entry != moduleList; entry = entry->Flink) {

        PVOID moduleEntry = (PVOID)entry;
        Log("[XM] Module[%d] entry: %p, moduleEntry: %p", index, entry, moduleEntry);

        ULONG_PTR dllbaseOffset = ntos.GetOffset("_KLDR_DATA_TABLE_ENTRY", "DllBase");
        ULONG_PTR sizeOfImageOffset = ntos.GetOffset("_KLDR_DATA_TABLE_ENTRY",
            "SizeOfImage");
        ULONG_PTR fullDllNameOffset = ntos.GetOffset("_KLDR_DATA_TABLE_ENTRY",
            "FullDllName");
        ULONG_PTR baseDllNameOffset = ntos.GetOffset("_KLDR_DATA_TABLE_ENTRY",
            "BaseDllName");

        Log("[XM]  DllBase=%p, SizeOfImage=%p, FullDllName=%p,BaseDllName = %p", dllbaseOffset, sizeOfImageOffset, fullDllNameOffset, baseDllNameOffset);

        /*
        PVOID dllBase = *(PVOID*)((ULONG_PTR)moduleEntry + dllbaseOffset);
        ULONG sizeOfImage = *(PULONG)((ULONG_PTR)moduleEntry + sizeOfImageOffset);

        PUNICODE_STRING fullDllName = (PUNICODE_STRING)((ULONG_PTR)moduleEntry +
            fullDllNameOffset);

        PUNICODE_STRING baseDllName = (PUNICODE_STRING)((ULONG_PTR)moduleEntry +
            baseDllNameOffset);
*/

//End
        index++;

        if (index > 200) {
            Log("[XM] Too many modules, break");
            break;
        }
    }

    return STATUS_SUCCESS;
}

/*

void CheckDriverMJHooked(PDRIVER_OBJECT DriverObj) {
    if (!DriverObj || !MmIsAddressValid(DriverObj)) {
        return;
    }

    __try {
        // 获取驱动的模块信息
        PVOID driverStart = DriverObj->DriverStart;
        ULONG driverSize = DriverObj->DriverSize;

        if (!driverStart || driverSize == 0) {
            Log("[XM]   No module info for driver: %wZ", &DriverObj->DriverName);
            return;
        }

        BOOLEAN hasHook = FALSE;
        ULONG hookCount = 0;

        Log("[XM]   Check MJFUNC for driver: %wZ (Base: %p, Size: 0x%x)",
            &DriverObj->DriverName, driverStart, driverSize);

        for (ULONG i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION && i < 28; i++) {
            PVOID majorFunc = DriverObj->MajorFunction[i];

            if (majorFunc) {
                // 检查函数地址是否在驱动模块范围内
                if ((ULONG_PTR)majorFunc < (ULONG_PTR)driverStart ||
                    (ULONG_PTR)majorFunc >= (ULONG_PTR)driverStart + driverSize) {

                    // 查找Hook函数所在的模块
                    CHAR hookModulePath[256] = { 0 };
                    PVOID hookImageBase = NULL;
                    ULONG hookImageSize = 0;

                    NTSTATUS findStatus = FindModuleByAddress(majorFunc, hookModulePath, &hookImageBase, &hookImageSize);

                    if (NT_SUCCESS(findStatus)) {
                        Log("[XM]  *** HOOK检测到 *** [%02d] %s = %p (在模块: %s, 基址: %p)",
                            i, majorFunctionNames[i], majorFunc, hookModulePath, hookImageBase);
                    }
                    else {
                        Log("[XM]  *** HOOK检测到 *** [%02d] %s = %p (未知模块)",
                            i, majorFunctionNames[i], majorFunc);
                    }

                    hasHook = TRUE;
                    hookCount++;
                }
                else {
                    Log("[XM] [%02d] %s = %p (normal)", i, majorFunctionNames[i], majorFunc);
                }
            }
            else {
                Log("[XM] [%02d] %s = NULL", i, majorFunctionNames[i]);
            }
        }

        if (hasHook) {
            Log("[XM] Driver %wZ: %d HOOKS DETECTED", &DriverObj->DriverName, hookCount);
        }
        else {
            Log("[XM] Driver %wZ: No hooks detected", &DriverObj->DriverName);
        }

    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        Log("[XM] CheckDriverMJHooked Exception %p", DriverObj);
    }
}

*/
