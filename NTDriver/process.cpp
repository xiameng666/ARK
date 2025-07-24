#include "process.h"

//NTKERNELAPI UCHAR* PsGetProcessImageFileName(PEPROCESS Process);

ENUM_PROCESS_META procMeta = { 0 };

void InitProcessPdb() {
    INIT_PDB;
    OFFSET(procMeta.EThreadToProcess,"_ETHREAD", "ThreadsProcess");
    OFFSET(procMeta.ProcessId,"_EPROCESS", "UniqueProcessId");
    OFFSET(procMeta.ActiveProcessLinks,"_EPROCESS", "ActiveProcessLinks");
    OFFSET(procMeta.ParentProcessId,"_EPROCESS", "InheritedFromUniqueProcessId");
    OFFSET(procMeta.ImageFileName,"_EPROCESS", "ImageFileName");
    ULONG pcbOffset, dtbOffset;
    OFFSET(pcbOffset, "_EPROCESS", "Pcb");
    OFFSET(dtbOffset, "_KPROCESS", "DirectoryTableBase");
    procMeta.DirectoryTableBase = pcbOffset + dtbOffset;
    procMeta.DirectoryTableBase = pcbOffset + dtbOffset;

    Log("[XM] Offsets: ETHREAD->EPROCESS=%x, PID=%x, Links=%x, PPID=%x, Name=%x, DTB=%x",
        procMeta.EThreadToProcess,
        procMeta.ProcessId,
        procMeta.ActiveProcessLinks,
        procMeta.ParentProcessId,
        procMeta.ImageFileName,
        procMeta.DirectoryTableBase);
}

NTSTATUS EnumProcessEx(PPROCESS_INFO processBuffer, bool onlyGetCount, PULONG processCount) {
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
