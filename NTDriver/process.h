#pragma once
#include"mydef.h"

extern "C" {

    //偏移量
    typedef struct ENUM_PROCESS_META {
        ULONG EThreadToProcess;     // ETHREAD -> EPROCESS
        ULONG ProcessId;           // UniqueProcessId 
        ULONG ActiveProcessLinks;  // ActiveProcessLinks  
        ULONG ParentProcessId;     // InheritedFromUniqueProcessId 
        ULONG ImageFileName;       // ImageFileName 
        ULONG DirectoryTableBase;  // CR3 
    }*PENUM_PROCESS_OFFSETS;

    //初始化偏移量
    void InitProcessPdb();

    //onlyGetCount 判断 获取进程数量还是遍历进程数据
    NTSTATUS EnumProcessFromLinksEx(PPROCESS_INFO processBuffer, BOOLEAN onlyGetCount, PULONG processCount);

    //PsLookupProcessByProcessId
    NTSTATUS EnumProcessByApiEx(PPROCESS_INFO ProcessInfos, BOOLEAN bCountOnly, PULONG pCount);

    //NTSTATUS EnumProcessBySearchMem
    
    NTSTATUS TerminateProcessByApi(HANDLE ProcessId);   //PspTerminateProcess

    NTSTATUS TerminateProcessByThread(HANDLE ProcessId);//清空进程的线程

    NTSTATUS AttachReadVirtualMem(HANDLE ProcessId, PVOID BaseAddress, PVOID Buffer, unsigned ReadBytes);

    NTSTATUS AttachWriteVirtualMem(HANDLE ProcessId, PVOID BaseAddress, PVOID Buffer, unsigned WriteBytes);

    NTSTATUS MemApiRead(HANDLE ProcessId, PVOID VirtualAddress, PVOID Buffer, SIZE_T Size);    //API读

    NTSTATUS MemApiWrite(HANDLE ProcessId, PVOID VirtualAddress, PVOID Buffer, SIZE_T Size);   //API写
}

EXTERN_C NTSTATUS MmCopyVirtualMemory(
    IN PEPROCESS FromProcess,
    IN CONST VOID* FromAddress,
    IN PEPROCESS ToProcess,
    OUT PVOID ToAddress,
    IN SIZE_T BufferSize,
    IN KPROCESSOR_MODE PreviousMode,
    OUT PSIZE_T NumberOfBytesCopied
);
