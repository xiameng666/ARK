#pragma once
#include "mydef.h"

extern "C" {

    NTSTATUS CheckDriverMJHookedForR3(PDISPATCH_HOOK_INFO HookBuffer, PULONG HookCount);

    void CheckDriverMJHooked(PDRIVER_OBJECT DriverObj);

    void EnumDriverObject();
}

// 全局驱动对象信息存储
typedef struct _DRIVER_OBJECT_INFO {
    PDRIVER_OBJECT DriverObject;
    WCHAR DriverName[128];      // 驱动名称
    PVOID DriverStart;          // 驱动基地址
    ULONG DriverSize;           // 驱动大小
} DRIVER_OBJECT_INFO, * PDRIVER_OBJECT_INFO;


enum _OBJECT_HEADER_Offset {
    _OBJECT_HEADER_Body_Offset = 0x30,
    _OBJECT_HEADER_TypeIndex_Offset = 0x18
};

//0x18 bytes (sizeof)
typedef struct _OBJECT_DIRECTORY_ENTRY
{
    struct _OBJECT_DIRECTORY_ENTRY* ChainLink;                              //0x0
    VOID* Object;                                                           //0x8
    ULONG HashValue;                                                        //0x10
}OBJECT_DIRECTORY_ENTRY, * POBJECT_DIRECTORY_ENTRY;

//0x158 bytes (sizeof)
typedef struct _OBJECT_DIRECTORY
{
    POBJECT_DIRECTORY_ENTRY HashBuckets[37];                                //0x0
    ULONG64 Lock;                                                           //0x128
    ULONG64 DeviceMap;                                                      //0x130
    struct _OBJECT_DIRECTORY* ShadowDirectory;                              //0x138
    VOID* NamespaceEntry;                                                   //0x140
    VOID* SessionObject;                                                    //0x148
    ULONG Flags;                                                            //0x150
    ULONG SessionId;                                                        //0x154
}OBJECT_DIRECTORY, * POBJECT_DIRECTORY;

//0x20 bytes (sizeof)
typedef struct _OBJECT_HEADER_NAME_INFO
{
    POBJECT_DIRECTORY Directory;                                            //0x0
    UNICODE_STRING Name;                                                    //0x8
    LONG ReferenceCount;                                                    //0x18
    ULONG Reserved;                                                         //0x1c
}OBJECT_HEADER_NAME_INFO, * POBJECT_HEADER_NAME_INFO;

static const char* majorFunctionNames[] = {
            "IRP_MJ_CREATE",
            "IRP_MJ_CREATE_NAMED_PIPE",
            "IRP_MJ_CLOSE",
            "IRP_MJ_READ",
            "IRP_MJ_WRITE",
            "IRP_MJ_QUERY_INFORMATION",
            "IRP_MJ_SET_INFORMATION",
            "IRP_MJ_QUERY_EA",
            "IRP_MJ_SET_EA",
            "IRP_MJ_FLUSH_BUFFERS",
            "IRP_MJ_QUERY_VOLUME_INFORMATION",
            "IRP_MJ_SET_VOLUME_INFORMATION",
            "IRP_MJ_DIRECTORY_CONTROL",
            "IRP_MJ_FILE_SYSTEM_CONTROL",
            "IRP_MJ_DEVICE_CONTROL",
            "IRP_MJ_INTERNAL_DEVICE_CONTROL",
            "IRP_MJ_SHUTDOWN",
            "IRP_MJ_LOCK_CONTROL",
            "IRP_MJ_CLEANUP",
            "IRP_MJ_CREATE_MAILSLOT",
            "IRP_MJ_QUERY_SECURITY",
            "IRP_MJ_SET_SECURITY",
            "IRP_MJ_POWER",
            "IRP_MJ_SYSTEM_CONTROL",
            "IRP_MJ_DEVICE_CHANGE",
            "IRP_MJ_QUERY_QUOTA",
            "IRP_MJ_SET_QUOTA",
            "IRP_MJ_PNP"
};
