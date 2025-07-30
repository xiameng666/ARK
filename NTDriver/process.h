#pragma once
#include"mydef.h"

extern "C" {

    // �������ƫ����
    typedef struct ENUM_PROCESS_META {
        ULONG EThreadToProcess;     // ETHREAD -> EPROCESS ƫ��
        ULONG ProcessId;           // UniqueProcessId ƫ��
        ULONG ActiveProcessLinks;  // ActiveProcessLinks ƫ��  
        ULONG ParentProcessId;     // InheritedFromUniqueProcessId ƫ��
        ULONG ImageFileName;       // ImageFileName ƫ��
        ULONG DirectoryTableBase;  // CR3 ƫ��
    }*PENUM_PROCESS_OFFSETS;

    void InitProcessPdb();

    //onlyGetCount �ж� ��ȡ�����������Ǳ�����������
    NTSTATUS EnumProcessFromLinksEx(PPROCESS_INFO processBuffer, bool onlyGetCount, PULONG processCount);

    //NTSTATUS EnumProcessBySearchMem
    
    //ǿ����ֹ����
    NTSTATUS TerminateProcessByApi(HANDLE ProcessId);   //PspTerminateProcess

    NTSTATUS TerminateProcessByThread(HANDLE ProcessId);//��ս��̵��߳�


}
