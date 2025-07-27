#pragma once
#include "driver.h"

// 枚举所有回调函数
void ForTest();
NTSTATUS EnumCallbacks(PCALLBACK_INFO callbackBuffer, CALLBACK_TYPE type, PULONG callbackCount);
NTSTATUS DeleteCallback(CALLBACK_TYPE type, ULONG index,PVOID deleteKey);

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

typedef struct _EX_CALLBACK_ROUTINE_BLOCK {
    EX_RUNDOWN_REF        RundownProtect;   // Rundown protection structure
    PEX_CALLBACK_FUNCTION Function;         // 回调函数地址
    PVOID                 Context;          // 回调上下文参数

} EX_CALLBACK_ROUTINE_BLOCK, * PEX_CALLBACK_ROUTINE_BLOCK;

//注册表回调结构 https://cloud.tencent.com/developer/article/2366837
typedef struct _CM_NOTIFY_ENTRY {
    LIST_ENTRY    ListEntryHead;        //链表项
    ULONG         UnKnown1;             
    ULONG         UnKnown2;             
    LARGE_INTEGER Cookie;               //回调标识符
    PVOID         Context;              //用户上下文
    PVOID         Function;             //回调函数地址
} CM_NOTIFY_ENTRY, *PCM_NOTIFY_ENTRY;
/*
//蓝屏回调结构
typedef struct _KBUGCHECK_CALLBACK_RECORD {
    LIST_ENTRY Entry;                   // +0x00 链表项  
    PKBUGCHECK_CALLBACK_ROUTINE CallbackRoutine;  // +0x10 回调函数
    PVOID Buffer;                       // +0x18 缓冲区
    ULONG Length;                       // +0x20 长度
    PUCHAR Component;                   // +0x28 组件名
    LARGE_INTEGER Checksum;             // +0x30 校验和
    UCHAR State;                        // +0x38 状态
} KBUGCHECK_CALLBACK_RECORD, *PKBUGCHECK_CALLBACK_RECORD;

typedef struct _KBUGCHECK_REASON_CALLBACK_RECORD {
    LIST_ENTRY Entry;                           // 链表项
    PKBUGCHECK_REASON_CALLBACK_ROUTINE CallbackRoutine;  // 回调函数
    PUCHAR Component;                           // 组件名
    ULONG_PTR Checksum;                        // 校验和
    KBUGCHECK_CALLBACK_REASON Reason;          // 回调原因
    UCHAR State;                               // 状态
} KBUGCHECK_REASON_CALLBACK_RECORD, * PKBUGCHECK_REASON_CALLBACK_RECORD;
*/

//关机回调结构
typedef struct _SHUTDOWN_PACKET {
    LIST_ENTRY ListEntry;               // +0x00 链表项
    PDEVICE_OBJECT DeviceObject;        // +0x10 设备对象
    PIRP Irp;                          // +0x18 IRP
} SHUTDOWN_PACKET, *PSHUTDOWN_PACKET;



