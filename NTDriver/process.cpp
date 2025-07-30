#include "process.h"

//NTKERNELAPI UCHAR* PsGetProcessImageFileName(PEPROCESS Process);

ENUM_PROCESS_META procMeta = { 0 };

void InitProcessPdb() {
    INIT_PDB;

    OFFSET(procMeta.EThreadToProcess, "_ETHREAD", "ThreadsProcess");
    OFFSET(procMeta.ProcessId, "_EPROCESS", "UniqueProcessId");
    OFFSET(procMeta.ActiveProcessLinks, "_EPROCESS", "ActiveProcessLinks");
    OFFSET(procMeta.ParentProcessId, "_EPROCESS", "InheritedFromUniqueProcessId");
    OFFSET(procMeta.ImageFileName, "_EPROCESS", "ImageFileName");

    // 初始化变量
    ULONG pcbOffset = 0, dtbOffset = 0;
    OFFSET(pcbOffset, "_EPROCESS", "Pcb");
    OFFSET(dtbOffset, "_KPROCESS", "DirectoryTableBase");
    procMeta.DirectoryTableBase = pcbOffset + dtbOffset;

    /*Log("[XM] Offsets: ETHREAD->EPROCESS=%x, PID=%x, Links=%x, PPID=%x, Name=%x, DTB=%x",
        procMeta.EThreadToProcess,
        procMeta.ProcessId,
        procMeta.ActiveProcessLinks,
        procMeta.ParentProcessId,
        procMeta.ImageFileName,
        procMeta.DirectoryTableBase);
    */
}

NTSTATUS EnumProcessFromLinksEx(PPROCESS_INFO processBuffer, bool onlyGetCount, PULONG processCount) {
    PEPROCESS CurrentProcess = NULL;
    PEPROCESS StartProcess = NULL;
    ULONG Counter = 0;

    __try {
        StartProcess = PsInitialSystemProcess;//指向系统进程的进程对象
        CurrentProcess = StartProcess;

        Log("[XM] 开始遍历进程，起始EPROCESS: %p ", CurrentProcess);

        do {
            if (!onlyGetCount && processBuffer != NULL) {
                PPROCESS_INFO pInfo = &processBuffer[Counter];
                RtlZeroMemory(pInfo, sizeof(PROCESS_INFO));

                pInfo->ProcessId = *(ULONG*)((PUCHAR)CurrentProcess + procMeta.ProcessId);
                pInfo->ParentProcessId = *(ULONG*)((PUCHAR)CurrentProcess + procMeta.ParentProcessId);
                pInfo->EprocessAddr = CurrentProcess;
                pInfo->DirectoryTableBase = *(ULONG*)((PUCHAR)CurrentProcess + procMeta.DirectoryTableBase);

                RtlCopyMemory(pInfo->ImageFileName, (PUCHAR)CurrentProcess + procMeta.ImageFileName, 15);
                pInfo->ImageFileName[15] = '\0';

                Log("[XM] 进程[%d]: PID=%d, Name=%s, EPROCESS=%p ",
                    Counter, pInfo->ProcessId, pInfo->ImageFileName, CurrentProcess);
            }

            Counter++;

            // 通过偏移获取ActiveProcessLinks
            PLIST_ENTRY pListEntry = (PLIST_ENTRY)((PUCHAR)CurrentProcess + procMeta.ActiveProcessLinks);
            PLIST_ENTRY pNextListEntry = pListEntry->Flink;
            CurrentProcess = (PEPROCESS)((PUCHAR)pNextListEntry - procMeta.ActiveProcessLinks);

        } while (CurrentProcess != StartProcess && Counter < 1000);

        *processCount = Counter;
        return STATUS_SUCCESS;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        Log("[XM] err EnumerateProcessCount");
        return STATUS_UNSUCCESSFUL;
    }
}



typedef NTSTATUS(*PspTerminateProcess)(PEPROCESS, NTSTATUS);

NTSTATUS TerminateProcessByApi(HANDLE ProcessId) {
    INIT_PDB;
    UNREFERENCED_PARAMETER(ProcessId);
    PspTerminateProcess pfn = (PspTerminateProcess)ntos.GetPointer("PspTerminateProcess");
    Log("[XM] PspTerminateProcess address: %p", pfn);

    if (!pfn) {
        Log("[XM] PspTerminateProcess not found");
        return STATUS_UNSUCCESSFUL;
    }
    /*
    PEPROCESS Process = NULL;
    NTSTATUS Status = PsLookupProcessByProcessId(ProcessId, &Process);
    if (!NT_SUCCESS(Status)) {
        Log("[XM] PsLookupProcessByProcessId fail PID %p: 0x%X", ProcessId, Status);
        return Status;
    }

    Status = pfn(Process, 0);
    ObDereferenceObject(Process);
    Log("[XM] PspTerminateProcess result: 0x%X", Status);
    */

    return STATUS_SUCCESS;
}

typedef NTSTATUS(NTAPI* PspTerminateThreadByPointer)(
    IN PETHREAD Thread,
    IN NTSTATUS ExitStatus,
    IN BOOLEAN bSelf
    );

NTSTATUS TerminateProcessByThread(HANDLE ProcessId)
{
    Log("[XM] TerminateProcessByThread: ProcessId = %p", ProcessId);

    INIT_PDB;
    PspTerminateThreadByPointer pfn = (PspTerminateThreadByPointer)ntos.GetPointer("PspTerminateThreadByPointer");

    if (!pfn) {
        Log("[XM] PspTerminateThreadByPointer GetPointer err");
        return STATUS_UNSUCCESSFUL;
    }

    ULONG Count = 0;

    //遍历所有线程
    for (int i = 0; i < 65536; i += 4) {
        PETHREAD Thread = NULL;
        NTSTATUS Status = PsLookupThreadByThreadId((HANDLE)i, &Thread);

        if (NT_SUCCESS(Status)) {
            PEPROCESS Process = IoThreadToProcess(Thread);
            HANDLE CurProcessId = PsGetProcessId(Process);

            if (CurProcessId == ProcessId) {
                Log("[XM] PspTerminateThreadByPointer Thread:%p\n", Thread);
                pfn(Thread, 0, FALSE);

                Count++;
            }
            ObDereferenceObject(Thread);
        }
    }

    return Count > 0 ? STATUS_SUCCESS : STATUS_NOT_FOUND;
}
