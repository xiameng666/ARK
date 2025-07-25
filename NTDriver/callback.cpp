#include "callback.h"


//extern "C" {
//    NTSTATUS PsRemoveCreateThreadNotifyRoutine(PCREATE_THREAD_NOTIFY_ROUTINE NotifyRoutine);
//    NTSTATUS PsRemoveLoadImageNotifyRoutine(PLOAD_IMAGE_NOTIFY_ROUTINE NotifyRoutine);
//}

// 好像内联了？ PVOID ExFastRefGetObject(  IN PEX_FAST_REF FastRef);

NTSTATUS EnumCallbacks(PCALLBACK_INFO callbackBuffer, CALLBACK_TYPE type, PULONG callbackCount) {
    if (!callbackBuffer || !callbackCount) {
        return STATUS_INVALID_PARAMETER;
    }
    
    *callbackCount = 0;
    ULONG count = 0;

    switch (type) {
        case TypeCreateProcess:
        case TypeCreateProcessEx:
        {
            INIT_PDB;
            ULONG_PTR processRoutineEntry = ntos.GetPointer("PspCreateProcessNotifyRoutine");
            Log("[XM] PspCreateProcessNotifyRoutine地址: %p", processRoutineEntry);

            PEX_CALLBACK callbackArray = (PEX_CALLBACK)processRoutineEntry;
            for (ULONG i = 0; i < 64; i++)
            {
                EX_FAST_REF* fastRef = &callbackArray[i].RoutineBlock;
                PVOID objectPtr = (PVOID)(fastRef->Value & ~0xF);  // 清除低4位
                PEX_CALLBACK_ROUTINE_BLOCK pObj = (PEX_CALLBACK_ROUTINE_BLOCK)objectPtr;
                if (pObj) {
                    //PVOID ctx = pObj->Context;
                    //PVOID funcAddr = pObj->Function;
                    //Log("[XM] 回调[%d]: 函数=%p, 上下文=%p", i, funcAddr, ctx);

                    PCALLBACK_INFO info = &callbackBuffer[count];
                    RtlZeroMemory(info, sizeof(CALLBACK_INFO));
                    info->Type = TypeCreateProcess;
                    info->Index = i;
                    info->CallbackEntry = pObj->Function;
                    info->IsValid = TRUE;

                    //CHAR ModulePath[256];
                    //PVOID CallbackExtra
                    count++;
                }
            }
        }
        break;

        case TypeCreateThread:
        
            break;

        case TypeLoadImage:
        
            break;

        case TypeRegistry:
        
            break;

        case TypeObject:
       
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

    PEX_CALLBACK callbackArray = (PEX_CALLBACK)processRoutineEntry;
    for (ULONG i = 0; i < 64; i++)
    {
        //ULONG_PTR refGetObjAddr = ntos.GetPointer("ExFastRefGetObject");
        //Log("[XM] ExFastRefGetObject地址: %p", refGetObjAddr);
        //PFNExFastRefGetObject pfnGetObj = (PFNExFastRefGetObject)refGetObjAddr;
        //PVOID objectPtr = pfnGetObj(&callbackArray[i].RoutineBlock);
        //Log("[XM] objectPtr:  %p", objectPtr);

        EX_FAST_REF* fastRef = &callbackArray[i].RoutineBlock;
        PVOID objectPtr = (PVOID)(fastRef->Value & ~0xF);  // 清除低4位
        PEX_CALLBACK_ROUTINE_BLOCK pObj = (PEX_CALLBACK_ROUTINE_BLOCK)objectPtr;
        if (pObj) {
            PVOID ctx = pObj->Context;
            PVOID funcAddr = pObj->Function;
            Log("[XM] 回调[%d]: 函数=%p, 上下文=%p", i, funcAddr, ctx);
        }
    }
}

// 删除回调函数
NTSTATUS DeleteCallback(CALLBACK_TYPE type, ULONG index) {
    Log("[XM] DeleteCallback: 回调删除功能，类型=%d，索引=%d", type, index);
    
    switch (type) {
        case TypeCreateProcess:
        case TypeCreateProcessEx:
            Log("[XM] 删除进程创建回调 [%d]", index);
            break;
            
        case TypeCreateThread:
            Log("[XM] 删除线程创建回调 [%d]", index);
            break;
            
        case TypeLoadImage:
            Log("[XM] 删除映像加载回调 [%d]", index);
            break;
            
        case TypeRegistry:
            Log("[XM] 删除注册表回调 [%d]", index);
            break;
            
        case TypeObject:
            Log("[XM] 删除对象回调 [%d]", index);
            break;
            
        default:
            Log("[XM] DeleteCallback: 不支持的回调类型 %d", type);
            return STATUS_NOT_SUPPORTED;
    }
    
    return STATUS_SUCCESS;
} 
