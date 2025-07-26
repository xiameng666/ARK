#include "callback.h"


//extern "C" {
//    NTSTATUS PsRemoveCreateThreadNotifyRoutine(PCREATE_THREAD_NOTIFY_ROUTINE NotifyRoutine);
//    NTSTATUS PsRemoveLoadImageNotifyRoutine(PLOAD_IMAGE_NOTIFY_ROUTINE NotifyRoutine);
//}

// 好像内联了？ PVOID ExFastRefGetObject(  IN PEX_FAST_REF FastRef);

#define GET_CALLBACK_FUNCTION_INFO(symbolName, WhatType)\
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
            info->Type = WhatType;\
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
        case TypeProcess:  // 合并了TypeCreateProcess和TypeCreateProcessEx
        {
            GET_CALLBACK_FUNCTION_INFO("PspCreateProcessNotifyRoutine", TypeProcess)
        }
        break;

        case TypeThread: 
        {
            GET_CALLBACK_FUNCTION_INFO("PspCreateThreadNotifyRoutine", TypeThread)
        }
        break;

        case TypeImage: 
        {
            GET_CALLBACK_FUNCTION_INFO("PspLoadImageNotifyRoutine", TypeImage)
        }
        break;

        case TypeRegistry:
        {
            GET_CALLBACK_FUNCTION_INFO("CmpCallBackVector", TypeRegistry)
        }
        case TypeObject:
        {
            //GET_CALLBACK_FUNCTION_INFO("ObpObjectCallbacks", TypeObject)
        }
        case TypeBugCheck:
        {
            //GET_CALLBACK_FUNCTION_INFO("KeBugCheckCallbackListHead", TypeBugCheck)  // 示例符号
        }
        case TypeShutdown:
        {
            //GET_CALLBACK_FUNCTION_INFO("IopShutdownNotifyList", TypeShutdown)
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
NTSTATUS DeleteCallback(CALLBACK_TYPE type, ULONG index, PVOID addr) {
    Log("[XM] DeleteCallback: 回调删除功能，类型=%d，索引=%d 地址=%p\n", type, index, addr);
    
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
            Log("[XM] 删除注册表回调 [%d]", index);
            break;
            
        case TypeObject:
            Log("[XM] 删除对象回调 [%d]", index);
            break;
            
        case TypeBugCheck:
            Log("[XM] 删除蓝屏回调 [%d]", index);
            break;
            
        case TypeShutdown:
            Log("[XM] 删除关机回调 [%d]", index);
            break;
            
        default:
            Log("[XM] DeleteCallback: 不支持的回调类型 %d", type);
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
