#pragma once
#include"mydef.h"

extern "C" {

    //ƫ����
    typedef struct ENUM_PROCESS_META {
        ULONG EThreadToProcess;     // ETHREAD -> EPROCESS
        ULONG ProcessId;           // UniqueProcessId 
        ULONG ActiveProcessLinks;  // ActiveProcessLinks  
        ULONG ParentProcessId;     // InheritedFromUniqueProcessId 
        ULONG ImageFileName;       // ImageFileName 
        ULONG DirectoryTableBase;  // CR3 
    }*PENUM_PROCESS_OFFSETS;

    //��ʼ��ƫ����
    void InitProcessPdb();

    //onlyGetCount �ж� ��ȡ�����������Ǳ�����������
    NTSTATUS EnumProcessFromLinksEx(PPROCESS_INFO processBuffer, BOOLEAN onlyGetCount, PULONG processCount);

    //PsLookupProcessByProcessId
    NTSTATUS EnumProcessByApiEx(PPROCESS_INFO ProcessInfos, BOOLEAN bCountOnly, PULONG pCount);

    //NTSTATUS EnumProcessBySearchMem
    
    NTSTATUS TerminateProcessByApi(HANDLE ProcessId);   //PspTerminateProcess

    NTSTATUS TerminateProcessByThread(HANDLE ProcessId);//��ս��̵��߳�


}
