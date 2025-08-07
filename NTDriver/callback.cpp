#include "callback.h"


//extern "C" {
//    NTSTATUS PsRemoveCreateThreadNotifyRoutine(PCREATE_THREAD_NOTIFY_ROUTINE NotifyRoutine);
//    NTSTATUS PsRemoveLoadImageNotifyRoutine(PLOAD_IMAGE_NOTIFY_ROUTINE NotifyRoutine);
//}

// 好像内联了？ PVOID ExFastRefGetObject(  IN PEX_FAST_REF FastRef);

//进程 线程 模块 存在数组
#define GET_CALLBACK_VECTOR_INFO(symbolName, enumType)\
    ULONG_PTR processRoutineEntry = ntos.GetPointer(symbolName);\
    Log("[XM] %s地址: %p", symbolName, processRoutineEntry);\
    PEX_CALLBACK callbackArray = (PEX_CALLBACK)processRoutineEntry;\
    for (ULONG i = 0; i < 64; i++)\
    {\
        EX_FAST_REF* fastRef = &callbackArray[i].RoutineBlock;\
        PVOID objectPtr = (PVOID)(fastRef->Value & ~0xF);\
        PEX_CALLBACK_ROUTINE_BLOCK pObj = (PEX_CALLBACK_ROUTINE_BLOCK)objectPtr;\
        if (pObj) {\
            PCALLBACK_INFO info = &callbackBuffer[count];\
            RtlZeroMemory(info, sizeof(CALLBACK_INFO));\
            info->Type = enumType;\
            info->Index = i;\
            info->CallbackEntry = pObj->Function;\
            info->IsValid = TRUE;\
            info->Extra.CallbackExtra = NULL;\
            FindModuleByAddress(pObj->Function, info->ModulePath, NULL, NULL);\
            Log("[XM] ModulePath:%s",info->ModulePath);\
            count++;\
        }\
    }

#define DELETE_CALLBACK_PFN(symbolName)\
    ULONG_PTR arrayBase = ntos.GetPointer(symbolName);\
    PEX_CALLBACK callbackArray = (PEX_CALLBACK)arrayBase;\
    EX_FAST_REF* fastRef = &callbackArray[index].RoutineBlock;\
    PVOID objectPtr = (PVOID)(fastRef->Value & ~0xF);\
    PEX_CALLBACK_ROUTINE_BLOCK block = (PEX_CALLBACK_ROUTINE_BLOCK)objectPtr;\
    PVOID originalFunction = block->Function;

NTSTATUS EnumCallbacks(PCALLBACK_INFO callbackBuffer, CALLBACK_TYPE type, PULONG callbackCount) {
    if (!callbackBuffer || !callbackCount) {
        return STATUS_INVALID_PARAMETER;
    }
    
    *callbackCount = 0;
    ULONG count = 0;

    EnumModule();
    INIT_NTOS;

    switch (type) {
        case TypeProcess: 
        {
            GET_CALLBACK_VECTOR_INFO("PspCreateProcessNotifyRoutine", TypeProcess)
        }
        break;

        case TypeThread: 
        {
            GET_CALLBACK_VECTOR_INFO("PspCreateThreadNotifyRoutine", TypeThread)
        }
        break;

        case TypeImage: 
        {
            GET_CALLBACK_VECTOR_INFO("PspLoadImageNotifyRoutine", TypeImage)
        }
        break;

        case TypeRegistry:
        {
            ULONG_PTR listHeadPtr = ntos.GetPointer("CallbackListHead");
            Log("[XM] CallbackListHead地址: %p", listHeadPtr);

            if (listHeadPtr) {
                PLIST_ENTRY listHead = (PLIST_ENTRY)listHeadPtr;
                PLIST_ENTRY currentEntry = listHead->Flink;

                while (currentEntry && currentEntry != listHead) {
                    PCM_NOTIFY_ENTRY callbackEntry = 
                        CONTAINING_RECORD(currentEntry, CM_NOTIFY_ENTRY, ListEntryHead);
                    
                    if (callbackEntry && callbackEntry->Function) {
                        PCALLBACK_INFO info = &callbackBuffer[count];
                        RtlZeroMemory(info, sizeof(CALLBACK_INFO));
                        info->Type = TypeRegistry;
                        info->Index = count;
                        info->CallbackEntry = callbackEntry->Function;
                        info->IsValid = TRUE;
                        info->Extra.CallbackExtra = (PVOID)callbackEntry->Cookie.QuadPart; // 保存Cookie值
                        FindModuleByAddress(callbackEntry->Function, info->ModulePath, NULL, NULL);
                        Log("[XM] Registry回调: %s, Cookie:0x%llx", info->ModulePath, callbackEntry->Cookie.QuadPart);
                        count++;
                    }
                    
                    currentEntry = currentEntry->Flink;
                    if (count >= 64) break;
                }
            }
        }
        break;

        case TypeObject:
        {
            ULONG_PTR ObTypeIndexTable = ntos.GetPointer("ObTypeIndexTable");
            
            if (!ObTypeIndexTable) {
                Log("[XM] 获取ObTypeIndexTable失败");
                break;
            }
            
            // 遍历所有对象类型
            for (int i = 0; i < 100 && count < 64; i++) {
                ULONG_PTR objTypeAddr = *(ULONG_PTR*)(ObTypeIndexTable + i * sizeof(ULONG_PTR));
                if (!objTypeAddr || !MmIsAddressValid((PVOID)objTypeAddr)) continue;
                
                // 获取对象类型名称
                size_t nameOffset = ntos.GetOffset("_OBJECT_TYPE", "Name");
                UNICODE_STRING* pName = (UNICODE_STRING*)(objTypeAddr + nameOffset);
                if (!MmIsAddressValid(pName) || !MmIsAddressValid(pName->Buffer)) continue;
                
                // 获取CallbackList链表
                size_t CallbackListOffset = ntos.GetOffset("_OBJECT_TYPE", "CallbackList");
                ULONG_PTR callbackListAddr = objTypeAddr + CallbackListOffset;
                LIST_ENTRY* head = (LIST_ENTRY*)callbackListAddr;
                
                // 检查链表是否为空
                if (!MmIsAddressValid(head->Flink) || head->Flink == head) {
                    continue; 
                }
               
                // 遍历CallbackList链表
                LIST_ENTRY* entry = head->Flink;
                int cbIndex = 0;
                
                while (entry != head && MmIsAddressValid(entry)) {
                    if (!MmIsAddressValid(entry->Flink)) {
                        Log("[XM] entry->Flink 无效，跳出循环");
                        break;
                    }
                    
                    CALLBACK_BODY* cb = CONTAINING_RECORD(entry, CALLBACK_BODY, ListEntry);

                    // 添加PreOp回调
                    if (cb->PreCallbackRoutine) {
                        PCALLBACK_INFO info = &callbackBuffer[count];
                        RtlZeroMemory(info, sizeof(CALLBACK_INFO));
                        
                        info->Type = TypeObject;
                        info->Index = count;
                        info->CallbackEntry = cb->PreCallbackRoutine;
                        info->IsValid = TRUE;
                        info->Extra.ObjectExtra.ObjTypeAddr = (PVOID)objTypeAddr; // 存储对象类型地址
                        info->Extra.ObjectExtra.CallbackRegistration = cb->CallbackNode; //删除回调需要的参数

                        // 复制对象类型名称到结构体
                        NTSTATUS status = RtlStringCbPrintfA(info->Extra.ObjectExtra.ObjectTypeName,
                            sizeof(info->Extra.ObjectExtra.ObjectTypeName),
                            "PreOb_%ws", pName->Buffer);
                        if (!NT_SUCCESS(status)) {
                            RtlStringCbCopyA(info->Extra.ObjectExtra.ObjectTypeName,
                                sizeof(info->Extra.ObjectExtra.ObjectTypeName), "PreOb");
                        }
                        
                        FindModuleByAddress(cb->PreCallbackRoutine, info->ModulePath, NULL, NULL);
                        Log("[XM] 对象回调 %ws PreOp[%d]: %p (%s)", pName->Buffer, cbIndex, 
                            cb->PreCallbackRoutine, info->ModulePath);
                        count++;
                    }
                    
                    // 添加PostOp回调
                    if (cb->PostCallbackRoutine) {
                        PCALLBACK_INFO info = &callbackBuffer[count];
                        RtlZeroMemory(info, sizeof(CALLBACK_INFO));
                        
                        info->Type = TypeObject;
                        info->Index = count;
                        info->CallbackEntry = cb->PostCallbackRoutine;
                        info->IsValid = TRUE;
                        info->Extra.ObjectExtra.ObjTypeAddr = (PVOID)objTypeAddr; // 存储对象类型地址
                        info->Extra.ObjectExtra.CallbackRegistration = cb->CallbackNode; //删除回调需要的参数、

                        // 复制对象类型名称到结构体
                        NTSTATUS status = RtlStringCbPrintfA(info->Extra.ObjectExtra.ObjectTypeName,
                            sizeof(info->Extra.ObjectExtra.ObjectTypeName),
                            "PostOb_%ws", pName->Buffer);
                        if (!NT_SUCCESS(status)) {
                            RtlStringCbCopyA(info->Extra.ObjectExtra.ObjectTypeName,
                                sizeof(info->Extra.ObjectExtra.ObjectTypeName), "PostOb");
                        }
                        
                        FindModuleByAddress(cb->PostCallbackRoutine, info->ModulePath, NULL, NULL);
                        Log("[XM] 对象回调 %ws PostOp[%d]: %p (%s)", pName->Buffer, cbIndex,
                            cb->PostCallbackRoutine, info->ModulePath);
                        count++;
                    }
                    
                    cbIndex++;
                    entry = entry->Flink;
                }
            }
            
            Log("[XM] 对象回调枚举完成，共找到 %d 个", count);
        }
        break;

        case TypeBugCheck:
        {
            ULONG_PTR listHeadPtr = ntos.GetPointer("KeBugCheckCallbackListHead");
            Log("[XM] KeBugCheckCallbackListHead地址: %p", listHeadPtr);
            
            if (listHeadPtr) {
                PLIST_ENTRY listHead = (PLIST_ENTRY)listHeadPtr;
                PLIST_ENTRY currentEntry = listHead->Flink;

                while (currentEntry && currentEntry != listHead) {
                    PKBUGCHECK_CALLBACK_RECORD callbackRecord = 
                        CONTAINING_RECORD(currentEntry, KBUGCHECK_CALLBACK_RECORD, Entry);
                    
                    if (callbackRecord && callbackRecord->CallbackRoutine) {
                        PCALLBACK_INFO info = &callbackBuffer[count];
                        RtlZeroMemory(info, sizeof(CALLBACK_INFO));
                        info->Type = TypeBugCheck;
                        info->Index = count;
                        info->CallbackEntry = callbackRecord->CallbackRoutine;
                        info->IsValid = TRUE;
                        info->Extra.CallbackExtra = (PVOID)callbackRecord; // 保存整个结构指针
                        FindModuleByAddress(callbackRecord->CallbackRoutine, info->ModulePath, NULL, NULL);
                        Log("[XM] BugCheck回调: %s", info->ModulePath);
                        count++;
                    }
                    
                    currentEntry = currentEntry->Flink;
                    if (count >= 64) break;
                }
            }
        }
        break;

        case TypeBugCheckReason:
        {
            ULONG_PTR listHeadPtr = ntos.GetPointer("KeBugCheckReasonCallbackListHead");
            Log("[XM] KeBugCheckReasonCallbackListHead地址: %p", listHeadPtr);
            
            if (listHeadPtr) {
                PLIST_ENTRY listHead = (PLIST_ENTRY)listHeadPtr;
                PLIST_ENTRY currentEntry = listHead->Flink;

                while (currentEntry && currentEntry != listHead) {
                    PKBUGCHECK_REASON_CALLBACK_RECORD callbackRecord = 
                        CONTAINING_RECORD(currentEntry, KBUGCHECK_REASON_CALLBACK_RECORD, Entry);
                    
                    if (callbackRecord && callbackRecord->CallbackRoutine) {
                        PCALLBACK_INFO info = &callbackBuffer[count];
                        RtlZeroMemory(info, sizeof(CALLBACK_INFO));
                        info->Type = TypeBugCheckReason;
                        info->Index = count;
                        info->CallbackEntry = callbackRecord->CallbackRoutine;
                        info->IsValid = TRUE;
                        info->Extra.CallbackExtra = (PVOID)callbackRecord; // 保存整个结构指针
                        FindModuleByAddress(callbackRecord->CallbackRoutine, info->ModulePath, NULL, NULL);
                        Log("[XM] BugCheckReason回调: %s", info->ModulePath);
                        count++;
                    }
                    
                    currentEntry = currentEntry->Flink;
                    if (count >= 64) break;
                }
            }
        }
        break;

        case TypeShutdown:
        {
            // 关机回调枚举
            ULONG_PTR listHeadPtr = ntos.GetPointer("IopNotifyShutdownQueueHead");
            Log("[XM] IopNotifyShutdownQueueHead地址: %p", listHeadPtr);
            
            if (listHeadPtr) {
                PLIST_ENTRY listHead = (PLIST_ENTRY)listHeadPtr;
                PLIST_ENTRY currentEntry = listHead->Flink;

                while (currentEntry && currentEntry != listHead) {
                    PSHUTDOWN_PACKET shutdownPacket = 
                        CONTAINING_RECORD(currentEntry, SHUTDOWN_PACKET, ListEntry);
                    
                    if (shutdownPacket && shutdownPacket->DeviceObject) {
                        
                        PCALLBACK_INFO info = &callbackBuffer[count];
                        RtlZeroMemory(info, sizeof(CALLBACK_INFO));
                        info->Type = TypeShutdown;
                        info->Index = count;
                        info->CallbackEntry = (PVOID)shutdownPacket->DeviceObject; // 设备对象作为标识
                        info->IsValid = TRUE;
                        info->Extra.CallbackExtra = (PVOID)shutdownPacket; // 保存整个结构指针
                        
                        // 使用驱动入口点来获取模块信息
                        PVOID driverStart = shutdownPacket->DeviceObject->DriverObject->DriverStart;
                        FindModuleByAddress(driverStart, info->ModulePath, NULL, NULL);
                        
                        Log("[XM] Shutdown回调: %s", info->ModulePath);
                        count++;
                    }
                    
                    currentEntry = currentEntry->Flink;
                    if (count >= 64) break;
                }
            }
        }
            break;

        default:
            return STATUS_NOT_SUPPORTED;
    }
        
    *callbackCount = count;
    Log("[XM] EnumCallbacks: 类型 %d, 返回 %d 个回调", type, count);
    return STATUS_SUCCESS;
       
}

void ForTest() {
    INIT_NTOS;
    /*
    // 首先获取对象回调相关的符号信息
    Log("[XM] ========== 开始获取对象回调相关符号信息 ==========");
    
    // 尝试获取各种可能的回调结构符号
    ULONG_PTR test1 = ntos.GetPointer("OBJECT_CALLBACK_ENTRY");
    ULONG_PTR test2 = ntos.GetPointer("OB_CALLBACK_BODY");  
    ULONG_PTR test3 = ntos.GetPointer("OB_CALLBACK_NODE");
    ULONG_PTR test4 = ntos.GetPointer("_CALLBACK_BODY");
    ULONG_PTR test5 = ntos.GetPointer("_CALLBACK_NODE");
    ULONG_PTR test6 = ntos.GetPointer("_OB_CALLBACK_BODY");
    ULONG_PTR test7 = ntos.GetPointer("_OB_CALLBACK_NODE");
    ULONG_PTR test8 = ntos.GetPointer("_OBJECT_CALLBACK_ENTRY");
    
    Log("[XM] OBJECT_CALLBACK_ENTRY 地址: %p", test1);
    Log("[XM] OB_CALLBACK_BODY 地址: %p", test2);
    Log("[XM] OB_CALLBACK_NODE 地址: %p", test3);  
    Log("[XM] _CALLBACK_BODY 地址: %p", test4);
    Log("[XM] _CALLBACK_NODE 地址: %p", test5);
    Log("[XM] _OB_CALLBACK_BODY 地址: %p", test6);
    Log("[XM] _OB_CALLBACK_NODE 地址: %p", test7);
    Log("[XM] _OBJECT_CALLBACK_ENTRY 地址: %p", test8);
    
    // 获取结构体大小和偏移信息
    Log("[XM] ========== 获取结构体偏移信息 ==========");
    
    // 尝试获取 CALLBACK_BODY 相关偏移
    size_t listEntryOffset = ntos.GetOffset("_CALLBACK_BODY", "ListEntry");
    size_t preOpOffset = ntos.GetOffset("_CALLBACK_BODY", "PreCallbackRoutine");
    size_t postOpOffset = ntos.GetOffset("_CALLBACK_BODY", "PostCallbackRoutine");
    size_t nodeOffset = ntos.GetOffset("_CALLBACK_BODY", "CallbackNode");
    
    Log("[XM] _CALLBACK_BODY.ListEntry 偏移: %p", listEntryOffset);
    Log("[XM] _CALLBACK_BODY.PreCallbackRoutine 偏移: %p", preOpOffset);
    Log("[XM] _CALLBACK_BODY.PostCallbackRoutine 偏移: %p", postOpOffset);
    Log("[XM] _CALLBACK_BODY.CallbackNode 偏移: %p", nodeOffset);
    
    // 尝试其他可能的结构名
    size_t obListEntry = ntos.GetOffset("OB_CALLBACK_BODY", "ListEntry");
    size_t obPreOp = ntos.GetOffset("OB_CALLBACK_BODY", "PreCallbackRoutine"); 
    size_t obPostOp = ntos.GetOffset("OB_CALLBACK_BODY", "PostCallbackRoutine");
    
    size_t ob2ListEntry = ntos.GetOffset("_OB_CALLBACK_BODY", "ListEntry");
    size_t ob2PreOp = ntos.GetOffset("_OB_CALLBACK_BODY", "PreCallbackRoutine");
    size_t ob2PostOp = ntos.GetOffset("_OB_CALLBACK_BODY", "PostCallbackRoutine");
    
    Log("[XM] OB_CALLBACK_BODY.ListEntry 偏移: %p", obListEntry);
    Log("[XM] OB_CALLBACK_BODY.PreCallbackRoutine 偏移: %p", obPreOp);
    Log("[XM] OB_CALLBACK_BODY.PostCallbackRoutine 偏移: %p", obPostOp);
    Log("[XM] _OB_CALLBACK_BODY.ListEntry 偏移: %p", ob2ListEntry);
    Log("[XM] _OB_CALLBACK_BODY.PreCallbackRoutine 偏移: %p", ob2PreOp);
    Log("[XM] _OB_CALLBACK_BODY.PostCallbackRoutine 偏移: %p", ob2PostOp);

    
    // 打印我们当前结构体的信息作为对比
    Log("[XM] ========== 当前结构体定义对比 ==========");
    Log("[XM] 我们的CALLBACK_BODY大小: %d", sizeof(CALLBACK_BODY));
    Log("[XM] 我们的CALLBACK_NODE大小: %d", sizeof(CALLBACK_NODE));
    Log("[XM] 我们的ListEntry偏移: %d", FIELD_OFFSET(CALLBACK_BODY, ListEntry));
    Log("[XM] 我们的PreCallbackRoutine偏移: %d", FIELD_OFFSET(CALLBACK_BODY, PreCallbackRoutine));
    Log("[XM] 我们的PostCallbackRoutine偏移: %d", FIELD_OFFSET(CALLBACK_BODY, PostCallbackRoutine));
        */

    ULONG_PTR ObTypeIndexTable = ntos.GetPointer("ObTypeIndexTable");
    Log("[XM] ObTypeIndexTable地址: %p", ObTypeIndexTable);

    //遍历 先打印出所有的对象名称看看
    int maxType = 100;
    for (int i = 0; i < maxType; i++) {
        // 取出POBJECT_TYPE指针
        ULONG_PTR objTypeAddr = *(ULONG_PTR*)(ObTypeIndexTable + i * sizeof(ULONG_PTR));
        if (!objTypeAddr) continue;

        // OBJECT_TYPE结构体Name字段偏移
        size_t nameoffset = ntos.GetOffset("_OBJECT_TYPE", "Name");
        // Log("[XM] nameoffset: %p", nameoffset);
        ULONG_PTR nameAddr = objTypeAddr + nameoffset;

        // 读取 UNICODE_STRING
        UNICODE_STRING* pName = (UNICODE_STRING*)nameAddr;
        if (!MmIsAddressValid(pName) || !MmIsAddressValid(pName->Buffer)) continue;

        // 打印对象类型名
        Log("[XM] 对象类型[%d] 地址: %p 名称: %ws", i, objTypeAddr, pName->Buffer);

        // 打印CallbackList地址
        size_t CallbackListOffset = ntos.GetOffset("_OBJECT_TYPE", "CallbackList");
        ULONG_PTR callbackListAddr = objTypeAddr + CallbackListOffset;
        Log("[XM] 对象类型[%d] CallbackList地址: %p", i, callbackListAddr);

        // 遍历CallbackList链表
        LIST_ENTRY* head = (LIST_ENTRY*)callbackListAddr;
        LIST_ENTRY* entry = head->Flink;
        int cbCount = 0;

        Log("[XM] 开始遍历CallbackList: head=%p head->Flink=%p head->Blink = % p",
            head, head->Flink, head->Blink);

        // 检查链表头的有效性
        if (!MmIsAddressValid(head->Flink)) {
            Log("[XM] 错误: head->Flink 无效地址 %p", head->Flink);
            continue;
        }

        // 检查是否为空链表
        if (head->Flink == head) {
            Log("[XM] 空链表，跳过");
            continue;
        }

        while (entry != head && MmIsAddressValid(entry)) {

            Log("[XM] 循环[%d] entry=%p", cbCount, entry);

            // 验证entry结构
            if (!MmIsAddressValid(entry->Flink)) {
                Log("[XM] entry->Flink 无效 %p", entry->Flink);
                break;
            }

            // 检查CALLBACK_BODY偏移
            ULONG_PTR cbAddr = (ULONG_PTR)entry - FIELD_OFFSET(CALLBACK_BODY,
                ListEntry);
            Log("[XM] 计算CALLBACK_BODY地址: %p", cbAddr);

            if (!MmIsAddressValid((PVOID)cbAddr)) {
                Log("[XM] CALLBACK_BODY 地址无效");
                break;
            }

            CALLBACK_BODY* cb = CONTAINING_RECORD(entry, CALLBACK_BODY, ListEntry);
            Log("[XM]   回调[%d] PreOp: %p PostOp: %p CallbackNode: %p", cbCount, cb->PreCallbackRoutine, cb->PostCallbackRoutine, cb->CallbackNode);

            //此处可以干点别的

            cbCount++;
            entry = entry->Flink;
        }
    }
}

// 删除回调函数
NTSTATUS DeleteCallback(CALLBACK_TYPE type, ULONG index, PVOID deleteKey) {
    Log("[XM] DeleteCallback: 回调删除功能，类型=%d，索引=%d 地址=%p\n", type, index, deleteKey);

    INIT_NTOS;

    switch (type) {
        case TypeProcess:
        {
            DELETE_CALLBACK_PFN("PspCreateProcessNotifyRoutine")
            NTSTATUS status = PsSetCreateProcessNotifyRoutine(
                (PCREATE_PROCESS_NOTIFY_ROUTINE)originalFunction,
                TRUE
            );
            return status;
        }
            
        case TypeThread:
        {
            DELETE_CALLBACK_PFN("PspCreateThreadNotifyRoutine")
            NTSTATUS status = PsRemoveCreateThreadNotifyRoutine((PCREATE_THREAD_NOTIFY_ROUTINE)originalFunction);
            return status;
        }
        case TypeImage:
        {
            DELETE_CALLBACK_PFN("PspLoadImageNotifyRoutine")
            NTSTATUS status = PsRemoveLoadImageNotifyRoutine((PLOAD_IMAGE_NOTIFY_ROUTINE)originalFunction);
            return status;
        }
        case TypeRegistry:
        {
            // 注册表回调删除使用Cookie，从Extra字段获取           
            LARGE_INTEGER cookie;
            cookie.QuadPart = (LONGLONG)deleteKey;  
            
            NTSTATUS status = CmUnRegisterCallback(cookie);
            Log("[XM] 删除注册表回调，Cookie=0x%llx，状态=0x%x", cookie.QuadPart, status);
            return status;
        }
            
        case TypeObject:
        {
            PVOID callbackRegistration = deleteKey;
            ObUnRegisterCallbacks(callbackRegistration);
            Log("[XM] 删除对象回调，CallbackRegistration=%p", callbackRegistration);
            return STATUS_SUCCESS;//不知道是否成功
        }        
            
        case TypeBugCheck:
        {
            // 蓝屏回调删除 需要整个结构          
            PKBUGCHECK_CALLBACK_RECORD callbackRecord = (PKBUGCHECK_CALLBACK_RECORD)deleteKey;
            NTSTATUS status = KeDeregisterBugCheckCallback(callbackRecord);
            Log("[XM] 删除蓝屏回调，状态=0x%x", status);
            return status;
        }

        case TypeBugCheckReason:
        {
            // 蓝屏原因回调删除 需要整个结构          
            PKBUGCHECK_REASON_CALLBACK_RECORD callbackRecord = (PKBUGCHECK_REASON_CALLBACK_RECORD)deleteKey;
            NTSTATUS status = KeDeregisterBugCheckReasonCallback(callbackRecord);
            Log("[XM] 删除蓝屏原因回调，状态=0x%x", status);
            return status;
        }
            
        case TypeShutdown:
        {
            // 关机回调  从链表中移除            
            PSHUTDOWN_PACKET shutdownPacket = (PSHUTDOWN_PACKET)deleteKey;

            ULONG_PTR listHeadPtr = ntos.GetPointer("IopNotifyShutdownQueueHead");
            if (listHeadPtr) {
                RemoveEntryList(&shutdownPacket->ListEntry);
                Log("[XM] 删除关机回调成功");
                return STATUS_SUCCESS;
            } else {
                Log("[XM] 删除关机回调失败：定位不到IopNotifyShutdownQueueHead");
                return STATUS_UNSUCCESSFUL;
            }
        }
            
        default:
            Log("[XM] DeleteCallback: default %d", type);
            return STATUS_NOT_SUPPORTED;
    }
    
    return STATUS_SUCCESS;
}


/*宏之前的 
        case TypeProcess:
        {
            INIT_NTOS;
            ULONG_PTR arrayBase = ntos.GetPointer("PspCreateProcessNotifyRoutine");
            PEX_CALLBACK callbackArray = (PEX_CALLBACK)arrayBase;
            EX_FAST_REF* fastRef = &callbackArray[index].RoutineBlock;
            PVOID objectPtr = (PVOID)(fastRef->Value & ~0xF);
            PEX_CALLBACK_ROUTINE_BLOCK block = (PEX_CALLBACK_ROUTINE_BLOCK)objectPtr;
            PVOID originalFunction = block->Function;

            NTSTATUS status = PsSetCreateProcessNotifyRoutine(
                (PCREATE_PROCESS_NOTIFY_ROUTINE)originalFunction,
                TRUE
            );

            return status;
        }

 */
