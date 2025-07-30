#pragma once
#include"mydef.h"

extern "C" {

    // 进程相关偏移量
    typedef struct ENUM_PROCESS_META {
        ULONG EThreadToProcess;     // ETHREAD -> EPROCESS 偏移
        ULONG ProcessId;           // UniqueProcessId 偏移
        ULONG ActiveProcessLinks;  // ActiveProcessLinks 偏移  
        ULONG ParentProcessId;     // InheritedFromUniqueProcessId 偏移
        ULONG ImageFileName;       // ImageFileName 偏移
        ULONG DirectoryTableBase;  // CR3 偏移
    }*PENUM_PROCESS_OFFSETS;

    void InitProcessPdb();

    //onlyGetCount 判断 获取进程数量还是遍历进程数据
    NTSTATUS EnumProcessFromLinksEx(PPROCESS_INFO processBuffer, bool onlyGetCount, PULONG processCount);

    //NTSTATUS EnumProcessBySearchMem
    
    //强制终止进程
    NTSTATUS TerminateProcessByApi(HANDLE ProcessId);   //PspTerminateProcess

    NTSTATUS TerminateProcessByThread(HANDLE ProcessId);//清空进程的线程


}
