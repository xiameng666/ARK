#include "process.h"

//NTKERNELAPI UCHAR* PsGetProcessImageFileName(PEPROCESS Process);

ENUM_PROCESS_META procMeta = { 0 };

void InitProcessPdb() {
    INIT_NTOS;

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
    INIT_NTOS;
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

    INIT_NTOS;
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

extern "C" NTSTATUS SeLocateProcessImageName(PEPROCESS Process, PUNICODE_STRING * ImagePath);

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

                    //拿进程路径
                    PUNICODE_STRING fullPathName = NULL;
                    NTSTATUS pathStatus = SeLocateProcessImageName(process, &fullPathName);
                    if (NT_SUCCESS(pathStatus)) {
                        SIZE_T copyLength = min(fullPathName->Length / sizeof(WCHAR), 
                                              sizeof(info->FullPathName) / sizeof(WCHAR) - 1);
                        wcsncpy(info->FullPathName, fullPathName->Buffer, copyLength);
                        info->FullPathName[copyLength] = L'\0';
                        
                        Log("[XM] Process path: %ws", info->FullPathName);
                        ExFreePool(fullPathName);
                    }
                    else {
                        info->FullPathName[0] = L'\0'; 
                    }
                
                

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

//内存特征EPROCESS
NTSTATUS EnumProcessBySearchMem(PPROCESS_INFO ProcessInfos, PULONG pCount)
{
    ULONG Count = 0;
    PEPROCESS systemProcess = NULL;

    PsLookupProcessByProcessId((HANDLE)4, &systemProcess);
    ULONG_PTR startAddr = (ULONG_PTR)systemProcess & ~0xFFFFFFF;
    ULONG_PTR endAddr = startAddr + 0x20000000;

    Log("startAddr:%p endAddr:%p", startAddr, endAddr);
    ObDereferenceObject(systemProcess);

    INIT_NTOS;
    size_t pcb_offset = ntos.GetOffset("_EPROCESS", "Pcb");
    size_t pid_offset = ntos.GetOffset("_EPROCESS", "UniqueProcessId");
    size_t imagename_offset = ntos.GetOffset("_EPROCESS", "ImageFileName");
    size_t header_type_offset = ntos.GetOffset("_KPROCESS", "Header") +
        ntos.GetOffset("_DISPATCHER_HEADER", "Type");//这个应该是0
    //size_t activelinks_offset = ntos.GetOffset("_EPROCESS", "ActiveProcessLinks");
    size_t ppid_offset = ntos.GetOffset("_EPROCESS", "InheritedFromUniqueProcessId");
    //size_t handletable_offset = ntos.GetOffset("_EPROCESS", "ObjectTable");
    size_t directory_table_offset = ntos.GetOffset("_KPROCESS", "DirectoryTableBase") + pcb_offset;
    size_t exittime_offset = ntos.GetOffset("_EPROCESS", "ExitTime");

    Log("pcb_offset:%p pid_offset:%p imagename_offset:%p header_type_offset:%p directory_table_offset:%p",
        pcb_offset, pid_offset, imagename_offset, header_type_offset, directory_table_offset);
        
    for (ULONG_PTR addr = startAddr; addr < endAddr; addr += 0x10) {
        PUCHAR eproc_bytes = (PUCHAR)addr;
        __try {
            //type是不是进程
            if (!MmIsAddressValid(eproc_bytes)) continue;
            UCHAR dispatcher_type = *(PUCHAR)addr;
            if (dispatcher_type != 0x03) {
                continue;
            }


            //pid
            if (!MmIsAddressValid(eproc_bytes + pid_offset)) continue;
            HANDLE pid = *(PHANDLE)(eproc_bytes + pid_offset);
            if ((ULONG_PTR)pid > 0x10000 || (ULONG_PTR)pid <4) {
                continue;
            }

            //ppid
            if (!MmIsAddressValid(eproc_bytes + ppid_offset)) continue;
            HANDLE ppid = *(PHANDLE)(eproc_bytes + ppid_offset);
            if ((ULONG_PTR)ppid > 0x10000) {
                continue;
            }

            //路径
            if (!MmIsAddressValid(eproc_bytes + imagename_offset)) continue;
            PCHAR imageName = (PCHAR)(eproc_bytes + imagename_offset);
            if (strlen(imageName) > 16) continue;

            /*
            //  ActiveProcessLinks
            if (!MmIsAddressValid(eproc_bytes + activelinks_offset)) continue;
            LIST_ENTRY* activeLinks = (LIST_ENTRY*)(eproc_bytes + activelinks_offset);
            if (activeLinks->Flink && MmIsAddressValid(activeLinks->Flink)) {
                if (activeLinks->Flink->Blink != activeLinks) {
                    continue; // 链表指针不一致，跳过
                }
            }
            */

            /*
            // HandleTable
            if (!MmIsAddressValid(eproc_bytes + handletable_offset)) continue;
            PVOID handleTable = *(PVOID*)(eproc_bytes + handletable_offset);
            if (!handleTable || !MmIsAddressValid(handleTable)) {
                continue; // HandleTable无效
            }
            if ((ULONG_PTR)handleTable < (ULONG_PTR)MmSystemRangeStart) {
                continue; // HandleTable不在内核地址空间
            }
            */

            //页目录表
            ULONG_PTR dtb = *(ULONG_PTR*)(eproc_bytes + directory_table_offset);
            if ((dtb & 0xFFF) != 0) continue;  //低12位为0
            if (dtb == 0 )  continue;

            // ExitTime 运行中的进程应该为0
            if (!MmIsAddressValid(eproc_bytes + exittime_offset)) continue;
            LARGE_INTEGER exitTime = *(PLARGE_INTEGER)(eproc_bytes + exittime_offset);
            if (exitTime.QuadPart != 0) {
                continue; 
            }

            /*
            //通过所有验证
            Log("[XM] ValidEPROCESS: PID=%d, Name=%s, EPROCESS=0x%p, HandleTable=0x%p ,dtb=%p",
                (ULONG)(ULONG_PTR)pid, imageName, addr, handleTable, dtb);
*/
            //复制到缓冲区
            PPROCESS_INFO pInfo = &ProcessInfos[Count];
            RtlZeroMemory(pInfo, sizeof(PROCESS_INFO));

            pInfo->EprocessAddr = (PEPROCESS)addr;

            pInfo->ProcessId = (ULONG)(ULONG_PTR)pid;

            RtlCopyMemory(pInfo->ImageFileName, imageName,sizeof(pInfo->ImageFileName));

            pInfo->ParentProcessId = (ULONG)(ULONG_PTR)ppid;

            pInfo->DirectoryTableBase = dtb;

            pInfo->FullPathName[0] = L'\0';
            
            // 通过PID查找PEPROCESS路径
            PEPROCESS validProcess = NULL;
            if (NT_SUCCESS(PsLookupProcessByProcessId((HANDLE)pid, &validProcess))) {
                PUNICODE_STRING fullPathName = NULL;
                NTSTATUS pathStatus = SeLocateProcessImageName(validProcess, &fullPathName);
                if (NT_SUCCESS(pathStatus) && fullPathName) {
                    SIZE_T copyLength = min(fullPathName->Length / sizeof(WCHAR),
                        sizeof(pInfo->FullPathName) / sizeof(WCHAR) - 1);
                    wcsncpy(pInfo->FullPathName, fullPathName->Buffer, copyLength);
                    pInfo->FullPathName[copyLength] = L'\0';
                    ExFreePool(fullPathName);
                }
                ObDereferenceObject(validProcess);
            }
            /*
            else {
                if (pid != (HANDLE)4) continue;
            }
            */

            Count++;

        }
        __except (1) {
            Log("[XM] LikeEPROCESS exception");
        }

    }

    *pCount = Count;
    Log("[XM] EnumProcessBySearchMem found %d processes", Count);
    return STATUS_SUCCESS;
}
