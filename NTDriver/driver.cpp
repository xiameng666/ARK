#include "driver.h"

#define MAX_OBJECT_STORE 500                     // 最多存储500个驱动对象
DRIVER_OBJECT_INFO g_DrvObjs[MAX_OBJECT_STORE]; 
ULONG g_DrvObjCount = 0;                         // 枚举得到的驱动数量
extern PDRIVER_OBJECT g_DriverObject ;


NTSTATUS EnumDeviceStackAttach(PDEVICE_STACK_INFO StackBuffer, PULONG StackCount) {

    EnumDriverObject();

    if (g_DrvObjCount == 0) {
        Log("[XM] CheckDeviceStack: No cached driver objects");
        return STATUS_UNSUCCESSFUL;
    }

    EnumModule();

    *StackCount = 0;
    for (ULONG i = 0; i < g_DrvObjCount && *StackCount < 2000; i++) {
        PDRIVER_OBJECT DriverObj = g_DrvObjs[i].DriverObject;

        if (!DriverObj || !MmIsAddressValid(DriverObj)) {
            continue;
        }

        PDEVICE_OBJECT CurrentDev = DriverObj->DeviceObject;

        if (CurrentDev && MmIsAddressValid(CurrentDev) && CurrentDev->AttachedDevice) {

            PDEVICE_STACK_INFO pStackInfo = &StackBuffer[*StackCount];
            RtlZeroMemory(pStackInfo, sizeof(DEVICE_STACK_INFO));

            //原始驱动对象 设备对象 
            pStackInfo->OrigDrvObj = (ULONG_PTR)DriverObj;
            pStackInfo->OrigDevObj = (ULONG_PTR)CurrentDev;
            pStackInfo->IsHooked = TRUE;

            Log("[XM] Driver[%lu]: Set OrigDrvObj=0x%p, OrigDevObj=0x%p", 
                i, (PVOID)pStackInfo->OrigDrvObj, (PVOID)pStackInfo->OrigDevObj);

            // 复制驱动名称
            size_t nameLen = wcslen(g_DrvObjs[i].DriverName);
            if (nameLen > 0 && nameLen < sizeof(pStackInfo->OrigDrvName) / sizeof(WCHAR)) {
                RtlCopyMemory(pStackInfo->OrigDrvName, g_DrvObjs[i].DriverName, 
                    (nameLen + 1) * sizeof(WCHAR));
                Log("[XM] Driver[%lu]: Copied driver name: %ws", i, pStackInfo->OrigDrvName);
            } 

            // 获取原始驱动路径并转换为UNICODE
            CHAR ansiPath[256] = { 0 };
            FindModuleByAddress(DriverObj->DriverStart, ansiPath, NULL, 0);
            
            // 将ANSI路径转换为UNICODE
            if (strlen(ansiPath) > 0) {
                ANSI_STRING ansiString;
                UNICODE_STRING unicodeString;
                RtlInitAnsiString(&ansiString, ansiPath);
                
                unicodeString.Buffer = pStackInfo->OriginalDriverPath;
                unicodeString.MaximumLength = sizeof(pStackInfo->OriginalDriverPath);
                unicodeString.Length = 0;
                
                NTSTATUS status = RtlAnsiStringToUnicodeString(&unicodeString, &ansiString, FALSE);
                if (!NT_SUCCESS(status)) {
                    pStackInfo->OriginalDriverPath[0] = L'\0';
                    Log("[XM] Warning: Failed to convert driver path to unicode");
                }
            } else {
                pStackInfo->OriginalDriverPath[0] = L'\0';
            }
            //Log("[XM] Found original driver path: %s", pStackInfo->OriginalDriverPath);

            // 遍历过滤驱动
            PDEVICE_OBJECT AttachedDev = CurrentDev->AttachedDevice;
            ULONG filterIndex = 0;

            while (AttachedDev && MmIsAddressValid(AttachedDev) && filterIndex < 8) {
                PDRIVER_OBJECT AttachedDriver = AttachedDev->DriverObject;

                if (MmIsAddressValid(AttachedDriver)) {
                    FILTER_DRIVER_INFO tempFilterInfo;
                    RtlZeroMemory(&tempFilterInfo, sizeof(FILTER_DRIVER_INFO));

                    //驱动对象 设备对象
                    tempFilterInfo.DriverObject = (ULONG_PTR)AttachedDriver;
                    tempFilterInfo.DeviceObject = (ULONG_PTR)AttachedDev;
                    
                    // 驱动名称 
                    if (AttachedDriver->DriverName.Buffer && AttachedDriver->DriverName.Length > 0) {
                         nameLen = AttachedDriver->DriverName.Length / sizeof(WCHAR);
                        if (nameLen < sizeof(tempFilterInfo.DriverName) / sizeof(WCHAR)) {
                            RtlCopyMemory(tempFilterInfo.DriverName, AttachedDriver->DriverName.Buffer, 
                                AttachedDriver->DriverName.Length);
                            tempFilterInfo.DriverName[nameLen] = L'\0';
                        } else {
                            // 如果名称过长，添加结束符
                            RtlCopyMemory(tempFilterInfo.DriverName, AttachedDriver->DriverName.Buffer, 
                                sizeof(tempFilterInfo.DriverName) - sizeof(WCHAR));
                            tempFilterInfo.DriverName[sizeof(tempFilterInfo.DriverName) / sizeof(WCHAR) - 1] = L'\0';
                            Log("[XM] EnumDeviceStackAttach 名称过长，添加结束符");
                        }
                    } else {
                        tempFilterInfo.DriverName[0] = L'\0';
                    }

                    Log("[XM] Filter[%lu]: DriverObject=0x%p, DeviceObject=0x%p, Name=%ws", 
                        filterIndex, (PVOID)tempFilterInfo.DriverObject, (PVOID)tempFilterInfo.DeviceObject, tempFilterInfo.DriverName);

                    // 模块路径查找
                    CHAR ansiFilterPath[256] = { 0 };
                    FindModuleByAddress(AttachedDriver->DriverStart, ansiFilterPath, NULL, 0);
                    
                    // 转换ANSI路径为UNICODE
                    if (strlen(ansiFilterPath) > 0) {
                        ANSI_STRING ansiString;
                        UNICODE_STRING unicodeString;
                        RtlInitAnsiString(&ansiString, ansiFilterPath);
                        
                        unicodeString.Buffer = tempFilterInfo.DriverPath;
                        unicodeString.MaximumLength = sizeof(tempFilterInfo.DriverPath);
                        unicodeString.Length = 0;
                        
                        NTSTATUS status = RtlAnsiStringToUnicodeString(&unicodeString, &ansiString, FALSE);
                        if (!NT_SUCCESS(status)) {
                            tempFilterInfo.DriverPath[0] = L'\0';
                            Log("[XM] Warning: Failed to convert filter driver path to unicode");
                        }
                    } else {
                        tempFilterInfo.DriverPath[0] = L'\0';
                    }

                    // 复制到目标缓冲区
                    pStackInfo->Filters[filterIndex] = tempFilterInfo;
                    filterIndex++;
                }

                AttachedDev = AttachedDev->AttachedDevice;
            }

            pStackInfo->FilterCount = filterIndex;
            
            Log("[XM] Driver[%lu]: Final OrigDrvObj=0x%p, OrigDevObj=0x%p", 
                i, (PVOID)pStackInfo->OrigDrvObj, (PVOID)pStackInfo->OrigDevObj);
            
            (*StackCount)++;
        }

        // CurrentDev = CurrentDev->NextDevice;
    }

    return STATUS_SUCCESS;
}

void EnumDriverObject() {
    INIT_NTOS;

    // 清空之前的枚举结果
    RtlZeroMemory(g_DrvObjs, sizeof(g_DrvObjs));
    g_DrvObjCount = 0;

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
        ULONG_PTR fullDllNameOffset = ntos.GetOffset("_LDR_DATA_TABLE_ENTRY", "FullDllName");
        // 遍历37个哈希表
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

                        PUCHAR ldrEntry = (PUCHAR)TargetDrvObj->DriverSection;
                        PUNICODE_STRING fullDllName = (PUNICODE_STRING)(ldrEntry + fullDllNameOffset);
                        if (fullDllName && fullDllName->Buffer) {
                            SIZE_T pathCopyLength = min(fullDllName->Length / sizeof(WCHAR),
                                sizeof(g_DrvObjs[g_DrvObjCount].DriverPath) /
                                sizeof(WCHAR) - 1);
                            wcsncpy(g_DrvObjs[g_DrvObjCount].DriverPath, fullDllName->Buffer,
                                pathCopyLength);
                            g_DrvObjs[g_DrvObjCount].DriverPath[pathCopyLength] = L'\0';

                            Log("[XM] Driver path: %ws", g_DrvObjs[g_DrvObjCount].DriverPath);
                        }
                        else {
                            g_DrvObjs[g_DrvObjCount].DriverPath[0] = L'\0';
                            Log("[XM] Failed to get FullDllName offset from PDB");
                        }
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


NTSTATUS EnumDrvMJHooked(PDISPATCH_HOOK_INFO HookBuffer, PULONG HookCount) {
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

