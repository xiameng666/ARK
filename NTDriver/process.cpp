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

NTSTATUS EnumProcessFromLinksEx(PPROCESS_INFO processBuffer, BOOLEAN onlyGetCount, PULONG processCount) {
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

NTSTATUS EnumProcessByApiEx(PPROCESS_INFO ProcessInfos, BOOLEAN bCountOnly, PULONG pCount) {
    Log("[XM] EnumProcessByApiEx 开始枚举进程，CountOnly=%d", bCountOnly);

    ULONG processCount = 0;
    NTSTATUS status = STATUS_SUCCESS;

    for (ULONG pid = 0; pid < 65536; pid += 4) {
        PEPROCESS process = NULL;

        NTSTATUS lookupStatus = PsLookupProcessByProcessId((HANDLE)(ULONG_PTR)pid, &process);

        if (NT_SUCCESS(lookupStatus) && process) {

            if (bCountOnly) {
                // 只计数
                processCount++;
            }
            else {
                if (ProcessInfos) {
                    // 填充进程信息
                    PROCESS_INFO* info = &ProcessInfos[processCount];
                    RtlZeroMemory(info, sizeof(PROCESS_INFO));

                    info->ProcessId = *(ULONG*)((PUCHAR)process + procMeta.ProcessId);
                    info->ParentProcessId = *(ULONG*)((PUCHAR)process + procMeta.ParentProcessId);
                    info->EprocessAddr = process;
                    info->DirectoryTableBase = *(ULONG*)((PUCHAR)process + procMeta.DirectoryTableBase);

                    // 名称
                    RtlCopyMemory(info->ImageFileName, (PUCHAR)process + procMeta.ImageFileName, 15);
                    info->ImageFileName[15] = '\0';

                    Log("[XM] 进程[%d]: PID=%d, Name=%s, EPROCESS=%p ",
                        processCount, info->ProcessId, info->ImageFileName, process);
                }

                processCount++;
            }

            ObDereferenceObject(process);
        }
    }

    *pCount = processCount;

    Log("[XM] EnumProcessByApiEx 共找到 %d 个进程", processCount);

    return status;
}

NTSTATUS AttachReadVirtualMem(HANDLE ProcessId, PVOID BaseAddress, PVOID Buffer, unsigned ReadBytes)
{
    PEPROCESS Process = NULL;
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    KAPC_STATE ApcState = { 0 };
    PHYSICAL_ADDRESS PhysicalAddress = { 0 };
    KIRQL  OldIrql = 0;

    Status = PsLookupProcessByProcessId(ProcessId, &Process);
    if (!NT_SUCCESS(Status)) {
        KdPrint(("[XM] AttachReadVirtualMem PsLookupProcessByProcessId Status:%08X\n", Status));
        return Status;
    }
    KdPrint(("[XM] AttachReadVirtualMem PEPROCESS:%p\n", Process));

    KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);
    KeStackAttachProcess(Process, &ApcState);//切换CR3

    PhysicalAddress = MmGetPhysicalAddress(BaseAddress);
    KdPrint(("[XM] AttachReadVirtualMem PhysicalAddress: 0x%08X\n", PhysicalAddress.LowPart));

    PVOID lpMapBase = MmMapIoSpace(PhysicalAddress, ReadBytes, MmNonCached);
    if (lpMapBase != NULL) {
        KdPrint(("[XM] AttachReadVirtualMem MmMapIoSpace lpMapBase: %p\n", lpMapBase));
        RtlCopyMemory(Buffer, lpMapBase, ReadBytes);
        MmUnmapIoSpace(lpMapBase, ReadBytes);
        Status = STATUS_SUCCESS;
    }
    else {
        KdPrint(("[XM] AttachReadVirtualMem MmMapIoSpace Failed\n"));
        MmUnmapIoSpace(lpMapBase, ReadBytes);
        return Status;
    }

    KeUnstackDetachProcess(&ApcState);
    KeLowerIrql(OldIrql);

    if (Process) {
        ObDereferenceObject(Process);
    }

    return Status;
}


NTSTATUS AttachWriteVirtualMem(HANDLE ProcessId, PVOID BaseAddress, PVOID Buffer, unsigned WriteBytes)
{
    PEPROCESS Process = NULL;
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    KAPC_STATE ApcState = { 0 };
    PHYSICAL_ADDRESS PhysicalAddress = { 0 };
    KIRQL  OldIrql = 0;

    Status = PsLookupProcessByProcessId(ProcessId, &Process);
    if (!NT_SUCCESS(Status)) {
        KdPrint(("[XM] AttachWriteVirtualMem PsLookupProcessByProcessId Status:%08X\n", Status));
        return Status;
    }
    KdPrint(("[XM] AttachWriteVirtualMem Process:%p\n", Process));

    KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);

    KeStackAttachProcess(Process, &ApcState);//切换CR3

    PhysicalAddress = MmGetPhysicalAddress(BaseAddress);

    //虚拟地址没有被映射为物理地址 不读了 
    if (PhysicalAddress.QuadPart == 0) {
        KdPrint(("[XM] AttachReadVirtualMem VA: %p (maps to physical 0x0)\n", BaseAddress));
        KeUnstackDetachProcess(&ApcState);
        KeLowerIrql(OldIrql);
        ObDereferenceObject(Process);
        return STATUS_INVALID_ADDRESS;
    }

    KdPrint(("[XM] AttachWriteVirtualMem PhysicalAddress: 0x%08X\n", PhysicalAddress.LowPart));

    PVOID lpMapBase = MmMapIoSpace(PhysicalAddress, WriteBytes, MmCached);
    if (lpMapBase != NULL) {
        RtlCopyMemory(lpMapBase, Buffer, WriteBytes);
        MmUnmapIoSpace(lpMapBase, WriteBytes);
        Status = STATUS_SUCCESS;
        KdPrint(("[XM] AttachWriteVirtualMem MmMapIoSpace Success: %p\n", lpMapBase));
    }
    else {
        KdPrint(("[XM] AttachWriteVirtualMem MmMapIoSpace Failed\n"));
        MmUnmapIoSpace(lpMapBase, WriteBytes);
        return Status;
    }

    KeUnstackDetachProcess(&ApcState);
    KeLowerIrql(OldIrql);

    if (Process) {
        ObDereferenceObject(Process);
    }

    return Status;
}

// 使用MmCopyVirtualMemory的API方式读取内存
NTSTATUS MemApiRead(HANDLE ProcessId, PVOID VirtualAddress, PVOID Buffer, SIZE_T Size)
{
    Log("[XM] MemApiRead: PID=%p, Addr=0x%p, Size=%zu", ProcessId, VirtualAddress, Size);
    
    PEPROCESS SourceProcess = NULL;
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    
    // 查找源进程对象
    Status = PsLookupProcessByProcessId(ProcessId, &SourceProcess);
    if (!NT_SUCCESS(Status)) {
        Log("[XM] MemApiRead: PsLookupProcessByProcessId failed: 0x%X", Status);
        return Status;
    }
    
   // ULONG ProcessIdValue = HandleToUlong(ProcessId);
    PEPROCESS TargetProcess = PsGetCurrentProcess();
    SIZE_T BytesRead = 0;
    
    __try {
        // 使用MmCopyVirtualMemory进行安全的跨进程内存复制
        Status = MmCopyVirtualMemory(
            SourceProcess,              // 源进程
            VirtualAddress,             // 源地址
            TargetProcess,              // 当前进程(驱动)
            Buffer,                     // 目标缓冲区
            Size,                       // 大小
            KernelMode,                 // 内核模式
            &BytesRead                  // 实际读取大小
        );
        
        if (NT_SUCCESS(Status)) {
            Log("[XM] MemApiRead: 成功读取 %zu 字节", BytesRead);
        } else {
            Log("[XM] MemApiRead: MmCopyVirtualMemory失败(0x%X)", Status);
        }
        
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        Status = STATUS_ACCESS_VIOLATION;
        Log("[XM] MemApiRead: 访问异常");
    }
    
    ObDereferenceObject(SourceProcess);
    return Status;
}

// 使用MmCopyVirtualMemory的API方式写入内存
NTSTATUS MemApiWrite(HANDLE ProcessId, PVOID VirtualAddress, PVOID Buffer, SIZE_T Size)
{
    Log("[XM] MemApiWrite: PID=%p, Addr=0x%p, Size=%zu", ProcessId, VirtualAddress, Size);
    
    PEPROCESS TargetProcess = NULL;
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    
    // 查找目标进程对象
    Status = PsLookupProcessByProcessId(ProcessId, &TargetProcess);
    if (!NT_SUCCESS(Status)) {
        Log("[XM] MemApiWrite: PsLookupProcessByProcessId failed: 0x%X", Status);
        return Status;
    }
    
    //ULONG ProcessIdValue = HandleToUlong(ProcessId);
    PEPROCESS SourceProcess = PsGetCurrentProcess();
    SIZE_T BytesWritten = 0;
    
    __try {
        // 使用MmCopyVirtualMemory进行安全的跨进程内存复制
        Status = MmCopyVirtualMemory(
            SourceProcess,              // 源进程(当前驱动)
            Buffer,                     // 源缓冲区
            TargetProcess,              // 目标进程
            VirtualAddress,             // 目标地址
            Size,                       // 大小
            KernelMode,                 // 内核模式
            &BytesWritten               // 实际写入大小
        );
        
        if (NT_SUCCESS(Status)) {
            Log("[XM] MemApiWrite: 成功写入 %zu 字节", BytesWritten);
        } else {
            Log("[XM] MemApiWrite: MmCopyVirtualMemory失败 Status(0x%X)", Status);
        }
        
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        Status = STATUS_ACCESS_VIOLATION;
        Log("[XM] MemApiWrite: 访问异常");
    }
    
    ObDereferenceObject(TargetProcess);
    return Status;
}
