#include "callback.h"


//extern "C" {
//    NTSTATUS PsRemoveCreateThreadNotifyRoutine(PCREATE_THREAD_NOTIFY_ROUTINE NotifyRoutine);
//    NTSTATUS PsRemoveLoadImageNotifyRoutine(PLOAD_IMAGE_NOTIFY_ROUTINE NotifyRoutine);
//}

// 好像内联了？ PVOID ExFastRefGetObject(  IN PEX_FAST_REF FastRef);

//进程 线程 模块 存在数组
#define GET_CALLBACK_VECTOR_INFO(symbolName, enumType)\
    INIT_PDB;\
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
    INIT_PDB;\
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
            INIT_PDB;
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
            Log("[XM] TypeObject ");
        }
        break;

        case TypeBugCheck:
        {
            INIT_PDB;
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
            INIT_PDB;
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
            INIT_PDB;
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
    INIT_PDB;
    ULONG_PTR processRoutineEntry = ntos.GetPointer("PspCreateProcessNotifyRoutine");
    Log("[XM] PspCreateProcessNotifyRoutine地址: %p", processRoutineEntry);

}

// 删除回调函数
NTSTATUS DeleteCallback(CALLBACK_TYPE type, ULONG index, PVOID deleteKey) {
    Log("[XM] DeleteCallback: 回调删除功能，类型=%d，索引=%d 地址=%p\n", type, index, deleteKey);
    
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
            Log("[XM] TypeObject");
            break;
            
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
            
            INIT_PDB;
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
            INIT_PDB;
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
