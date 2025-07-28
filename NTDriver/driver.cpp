#include "driver.h"

#define MAX_OBJECT_STORE 500                           // 最多存储500个驱动对象
DRIVER_OBJECT_INFO g_DrvObjs[MAX_OBJECT_STORE]; 
ULONG g_DrvObjCount = 0;                         // 枚举得到的驱动数量
extern PDRIVER_OBJECT g_DriverObject ;


NTSTATUS CheckDeviceStack(PDEVICE_STACK_INFO StackBuffer, PULONG StackCount) {
    EnumDriverObject();
    *StackCount = 0;

    if (g_DrvObjCount == 0) {
        Log("[XM] CheckDeviceStack: No cached driver objects");
        return STATUS_UNSUCCESSFUL;
    }

    EnumModule();

    for (ULONG i = 0; i < g_DrvObjCount && *StackCount < 2000; i++) {
        PDRIVER_OBJECT DriverObj = g_DrvObjs[i].DriverObject;

        if (!DriverObj || !MmIsAddressValid(DriverObj)) {
            continue;
        }

        PDEVICE_OBJECT CurrentDev = DriverObj->DeviceObject;

        if (CurrentDev && MmIsAddressValid(CurrentDev) && CurrentDev->AttachedDevice) {
            PDEVICE_STACK_INFO pStackInfo = &StackBuffer[*StackCount];
            RtlZeroMemory(pStackInfo, sizeof(DEVICE_STACK_INFO));

            // 复制驱动名称
            size_t copyLength = min(wcslen(g_DrvObjs[i].DriverName) * sizeof(WCHAR),
                sizeof(pStackInfo->OrigDrvName) - sizeof(WCHAR));
            RtlCopyMemory(pStackInfo->OrigDrvName, g_DrvObjs[i].DriverName, copyLength);
            pStackInfo->OrigDrvName[copyLength / sizeof(WCHAR)] = L'\0';

            //原始驱动对象 设备对象
            pStackInfo->OrigDrvObg = DriverObj;
            pStackInfo->OrigDevObj = CurrentDev;
            pStackInfo->IsHooked = TRUE;

            // 获取原始驱动路径
            FindModuleByAddress(DriverObj->DriverStart,
                pStackInfo->OriginalDriverPath, NULL, 0);
            Log("[XM] Found original driver path: %s", pStackInfo->OriginalDriverPath);

            // 遍历过滤驱动
            PDEVICE_OBJECT AttachedDev = CurrentDev->AttachedDevice;
            ULONG filterIndex = 0;

            while (AttachedDev&&MmIsAddressValid(AttachedDev)) {
                PDRIVER_OBJECT AttachedDriver = AttachedDev->DriverObject;

                if (MmIsAddressValid(AttachedDriver)) {
                    PFILTER_DRIVER_INFO filterInfo = &pStackInfo->Filters[filterIndex];
                    RtlZeroMemory(filterInfo, sizeof(FILTER_DRIVER_INFO));

                    // 驱动名称
                    copyLength = min(AttachedDriver->DriverName.Length,
                        sizeof(filterInfo->DriverName) - sizeof(WCHAR));
                    RtlCopyMemory(filterInfo->DriverName, AttachedDriver->DriverName.Buffer, copyLength);
                    filterInfo->DriverName[copyLength / sizeof(WCHAR)] = L'\0';

                    //驱动对象 设备对象
                    filterInfo->DriverObject = AttachedDriver;
                    filterInfo->DeviceObject = AttachedDev;

                    // 模块路径查找
                    FindModuleByAddress(AttachedDriver->DriverStart,
                        filterInfo->DriverPath, NULL, 0);

                    filterIndex++;
                }

                AttachedDev = AttachedDev->AttachedDevice;
            }

            pStackInfo->FilterCount = filterIndex;
            (*StackCount)++;
        }

        // CurrentDev = CurrentDev->NextDevice;
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
                    if (g_DrvObjCount < MAX_OBJECT_STORE) {
                        g_DrvObjs[g_DrvObjCount].DriverObject = TargetDrvObj;

                        // 驱动名称
                        ULONG copyLength = min(TargetDrvObj->DriverName.Length, sizeof(g_DrvObjs[g_DrvObjCount].DriverName) - sizeof(WCHAR));
                        RtlCopyMemory(g_DrvObjs[g_DrvObjCount].DriverName,
                            TargetDrvObj->DriverName.Buffer, copyLength);
                        g_DrvObjs[g_DrvObjCount].DriverName[copyLength / sizeof(WCHAR)] = L'\0';

                        // 驱动基地址和大小
                        g_DrvObjs[g_DrvObjCount].DriverStart = TargetDrvObj->DriverStart;
                        g_DrvObjs[g_DrvObjCount].DriverSize = TargetDrvObj->DriverSize;

                        g_DrvObjCount++;
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


NTSTATUS CheckDrvMJHooked(PDISPATCH_HOOK_INFO HookBuffer, PULONG HookCount) {
    EnumDriverObject();

    *HookCount = 0;

    if (g_DrvObjCount == 0) {
        Log("[XM] CheckDrvMJHooked: No cached driver objects");
        return STATUS_UNSUCCESSFUL;
    }

    EnumModule();

    CHAR ansiDriverNameBuffer[256] = { 0 };

    // 遍历存储的驱动对象
    for (ULONG i = 0; i < g_DrvObjCount; i++) {

        PDRIVER_OBJECT DriverObj = g_DrvObjs[i].DriverObject;

        if (!DriverObj || !MmIsAddressValid(DriverObj)) {
            continue;
        }

        //PWCHAR driverName = g_DrvObjs[i].DriverName;

        __try {
            PVOID driverStart = DriverObj->DriverStart;
            ULONG driverSize = DriverObj->DriverSize;

            // 检查所有MajorFunction 
            for (ULONG j = 0; j <= 27; j++) {

                if (!MmIsAddressValid(DriverObj->MajorFunction[j])) {
                    continue;
                }

                PVOID majorFunc = DriverObj->MajorFunction[j];

                BOOLEAN isHooked = FALSE;

                // NULL指针  正常
                if (!majorFunc) {
                    continue;
                }

                CHAR modulePath[256] = { 0 };
                PVOID moduleBase = NULL;
                ULONG moduleSize = 0;

                NTSTATUS findStatus = FindModuleByAddress(majorFunc, modulePath, &moduleBase, &moduleSize);
                if (NT_SUCCESS(findStatus)) {
                    // 指向ntoskrnl.exe
                    if (strstr(modulePath, "ntoskrnl.exe")) {
                        continue;
                    }

                    // 指向驱动自身模块
                    if ((ULONG_PTR)majorFunc >= (ULONG_PTR)driverStart &&
                        (ULONG_PTR)majorFunc < (ULONG_PTR)driverStart + driverSize) {
                        continue;
                    }

                    // 指向其他模块
                    isHooked = TRUE;
                }

                if (!isHooked) continue;

                if (*HookCount >= 800) {
                    Log("[XM] CheckDrvMJHooked: Reached safety limit of 800 entries to avoid buffer overflow");
                    return STATUS_SUCCESS;
                }

                PDISPATCH_HOOK_INFO pHookInfo = &HookBuffer[*HookCount];
                if (!MmIsAddressValid(pHookInfo)) {
                    Log("[XM] CheckDrvMJHooked: Invalid pHookInfo address at index %d", *HookCount);
                    return STATUS_INVALID_PARAMETER;
                }

                RtlZeroMemory(pHookInfo, sizeof(DISPATCH_HOOK_INFO));
                RtlZeroMemory(ansiDriverNameBuffer, sizeof(ansiDriverNameBuffer));
                pHookInfo->MajorFunctionCode = j;

                RtlStringCbCopyA(pHookInfo->FunctionName, sizeof(pHookInfo->FunctionName),
                    majorFunctionNames[j]);

                ANSI_STRING ansiDriverName = { 0 };
                ansiDriverName.Buffer = ansiDriverNameBuffer;
                ansiDriverName.MaximumLength = 255;

                UNICODE_STRING unicodeDriverName;
                RtlInitUnicodeString(&unicodeDriverName, g_DrvObjs[i].DriverName);

                RtlUnicodeStringToAnsiString(&ansiDriverName, &unicodeDriverName, FALSE);
                RtlStringCbCopyA(pHookInfo->DriverName, sizeof(pHookInfo->DriverName), ansiDriverName.Buffer);

                pHookInfo->CurrentAddress = majorFunc;
                pHookInfo->IsHooked = isHooked;

                if (NT_SUCCESS(findStatus)) {
                    RtlStringCbCopyA(pHookInfo->CurrentModule, sizeof(pHookInfo->CurrentModule), modulePath);
                }
                else {
                    RtlStringCbCopyA(pHookInfo->CurrentModule, sizeof(pHookInfo->CurrentModule), "Unknown");
                }

                (*HookCount)++;


                // 限制最大检测数量避免SystemBuffer溢出
                if (*HookCount >= 600) {
                    Log("[XM] CheckDrvMJHooked: Reached safety limit of 800 entries to avoid buffer overflow");
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
