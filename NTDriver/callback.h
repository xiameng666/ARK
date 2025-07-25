#pragma once
#include "driver.h"

// 枚举所有回调函数
void ForTest();
NTSTATUS EnumCallbacks(PCALLBACK_INFO callbackBuffer, CALLBACK_TYPE type, PULONG callbackCount);
NTSTATUS DeleteCallback(CALLBACK_TYPE type, ULONG index);

typedef struct _EX_FAST_REF {
    union {
        PVOID Object;          // 对象指针 指向 EX_CALLBACK_ROUTINE_BLOCK
        ULONG_PTR RefCnt : 4;  // 引用计数 (x86是3位)
        ULONG_PTR Value;       // 上述两者的组合值
    };
} EX_FAST_REF, * PEX_FAST_REF;

//数组中的每个元素
typedef struct _EX_CALLBACK {
    EX_FAST_REF RoutineBlock;   // 指向实际的回调数据
} EX_CALLBACK, * PEX_CALLBACK;

//#define PSP_MAX_CREATE_PROCESS_NOTIFY 8
//EX_CALLBACK PspCreateProcessNotifyRoutine[PSP_MAX_CREATE_PROCESS_NOTIFY];

// win10+ 回调块结构
typedef struct _EX_CALLBACK_ROUTINE_BLOCK {
    EX_RUNDOWN_REF        RundownProtect;   // Rundown protection structure
    PEX_CALLBACK_FUNCTION Function;         // 回调函数地址
    PVOID                 Context;          // 回调上下文参数
} EX_CALLBACK_ROUTINE_BLOCK, * PEX_CALLBACK_ROUTINE_BLOCK;
