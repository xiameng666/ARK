#include "ArkR3.h"


// extern SSDT_INFO g_SSDT_XP_SP3_Table[];

// 构造函数
ArkR3::ArkR3() : memBuffer_(nullptr), memBufferSize_(0), memDataSize_(0)
{
    // 初始化时分配一个基础大小的缓冲区
    MemEnsureBufferSize(4096); // 初始4KB
}

ArkR3::~ArkR3()
{
    if (memBuffer_) {
        free(memBuffer_);
        memBuffer_ = nullptr;
    }
    memBufferSize_ = 0;
    memDataSize_ = 0;
}

// 从ezpdb获取PDB路径并设置给驱动
bool ArkR3::SetPdbPathFromEzpdb() {
    if (!ntos_pdb_) {
        Log("ezpdb对象未初始化\n");
        return false;
    }

    // 从ezpdb获取当前PDB路径
    std::string pdbPathA = ntos_pdb_->get_current_pdb_path();
    if (pdbPathA.empty()) {
        Log("无法从ezpdb获取PDB路径\n");
        return false;
    }

    // 转换为Unicode（路径是全英文）
    wchar_t pdbPathW[MAX_PATH] = { 0 };
    size_t len = strlen(pdbPathA.c_str());
    for (size_t i = 0; i < len && i < MAX_PATH - 1; ++i) {
        pdbPathW[i] = (wchar_t)pdbPathA[i];
    }

    PDB_PATH_REQUEST pdbPathReq = { 0 };
    wcscpy_s(pdbPathReq.DownloadPath, pdbPathW);

    DWORD bytesReturned = 0;
    BOOL result = DeviceIoControl(
        m_hDriver,
        CTL_SET_PDB_PATH,
        &pdbPathReq,
        sizeof(pdbPathReq),
        NULL,
        0,
        &bytesReturned,
        NULL
    );

    if (result) {
        Log("SetPdbPathFromEzpdb: %s\n", pdbPathA.c_str());
        return true;
    }
    else {
        Log("SetPdbPathFromEzpdb err\n");
        return false;
    }
}

bool ArkR3::InitSymbolState()
{
    // 获取ntoskrnl.exe路径
    char* sysroot = nullptr;
    size_t len = 0;
    if (_dupenv_s(&sysroot, &len, "systemroot") != 0 || !sysroot) return false;
    ntos_path_ = std::string(sysroot) + "\\System32\\ntoskrnl.exe";
    free(sysroot);
    // 获取ntoskrnl.exe基址
    ntbase_ = GetModuleBase("ntoskrnl.exe");
    if (!ntbase_) return false;

    // 创建并初始化PDB对象
    ntos_pdb_ = std::make_unique<ez::pdb>(ntos_path_, "http://msdl.blackint3.com:88/download/symbols/");
    if (!ntos_pdb_->init()) {
        ntos_pdb_.reset();
        return false;
    }
    return true;
}

BOOL ArkR3::SendVA(ULONG_PTR va)
{
    if (m_hDriver == INVALID_HANDLE_VALUE) {
        Log("SendVA: err");
        return FALSE;
    }
    DWORD written = 0;
    BOOL bResult = WriteFile(m_hDriver, &va, sizeof(va), &written, NULL);
    if (bResult && written == sizeof(va)) {
        Log("SendVA: 发送VA=0x%p\n", va);
        return TRUE;
    }
    else {
        LogErr("SendVA: 发送失败\n");
        return FALSE;
    }
}

typedef NTSTATUS(NTAPI* PFN_NtQuerySystemInformation)(
    SYSTEM_INFORMATION_CLASS,
    PVOID,
    ULONG,
    PULONG
    );

#define SystemModuleInformation ((SYSTEM_INFORMATION_CLASS)11)

ULONG_PTR ArkR3::GetModuleBase(const char* moduleName)
{
    HMODULE hNtdll = GetModuleHandleA("ntdll.dll");
    if (!hNtdll) return 0;
    PFN_NtQuerySystemInformation NtQuerySystemInformation =
        (PFN_NtQuerySystemInformation)GetProcAddress(hNtdll, "NtQuerySystemInformation");
    if (!NtQuerySystemInformation) return 0;

    ULONG len = 0;
    NTSTATUS status = NtQuerySystemInformation(SystemModuleInformation, nullptr, 0, &len);
    if (status != 0xC0000004L) // STATUS_INFO_LENGTH_MISMATCH
        return 0;

    BYTE* buffer = (BYTE*)malloc(len);
    if (!buffer)
        return 0;

    status = NtQuerySystemInformation(SystemModuleInformation, buffer, len, &len);
    if (status < 0) {
        free(buffer);
        return 0;
    }

    PRTL_PROCESS_MODULES modules = (PRTL_PROCESS_MODULES)buffer;
    for (ULONG i = 0; i < modules->NumberOfModules; ++i) {
        PRTL_PROCESS_MODULE_INFORMATION p = &modules->Modules[i];
        const char* name = (const char*)p->FullPathName + p->OffsetToFileName;
        if (_stricmp(name, moduleName) == 0) {
            Log("[symbol]获取%s基址 0x%p\n", moduleName, p->ImageBase);
            ULONG_PTR base = (ULONG_PTR)p->ImageBase;
            free(buffer);
            return base;
        }
    }
    free(buffer);
    return 0;
}

ULONG_PTR ArkR3::GetKernelSymbolVA(const char* symbolName)
{
    if (!ntos_pdb_)
        return 0;
    int rva = ntos_pdb_->get_rva(symbolName);
    if (rva < 0)
        return 0;
    if (!ntbase_)
        return 0;
    ULONG_PTR va = ntbase_ + rva;
    Log("[symbol] %s 的VA: 0x%p\n", symbolName, (void*)va);
    return va;
}

ULONG ArkR3::GetKernelSymbolOffset(const char* structName, const wchar_t* fieldName)
{
    if (!ntos_pdb_)
        return 0;

    int offset = ntos_pdb_->get_attribute_offset(structName, fieldName);
    if (offset < 0)
        return 0;

    Log("[symbol] %s.%ws 的偏移: 0x%x\n", structName, fieldName, offset);
    return (ULONG)offset;
}

// 确保缓冲区大小足够
BOOL ArkR3::MemEnsureBufferSize(DWORD requiredSize)
{
    if (requiredSize > 0x100000) { // 限制最大1MB
        Log("EnsureBufferSize: Size too large (%d bytes)\n", requiredSize);
        return FALSE;
    }

    if (memBufferSize_ >= requiredSize) {
        return TRUE; // 当前缓冲区已足够
    }

    // 计算新的缓冲区大小（向上取整到4KB边界）
    DWORD newSize = ((requiredSize + 4095) / 4096) * 4096;

    PVOID newBuffer = realloc(memBuffer_, newSize);
    if (!newBuffer) {
        Log("EnsureBufferSize: Failed to allocate %d bytes\n", newSize);
        return FALSE;
    }

    memBuffer_ = newBuffer;
    memBufferSize_ = newSize;

    Log("EnsureBufferSize: Buffer resized to %d bytes\n", newSize);
    return TRUE;
}

void ArkR3::MemClearBuffer()
{
    if (memBuffer_ && memBufferSize_ > 0) {
        memset(memBuffer_, 0, memBufferSize_);
    }
    memDataSize_ = 0;
}


PSEGDESC ArkR3::GDTGetSingle(UINT cpuIndex, PGDTR pGdtr,DWORD* pRetBytes)
{
    DWORD gdtSize = 0x1000; // 固定大缓冲区
    PSEGDESC pBuffer = (PSEGDESC)malloc(gdtSize);
    if (!pBuffer) {
        LogErr("GetSingeGDT malloc err\n");
        return nullptr;
    }

    GDT_DATA_REQ req = { 0 };
    req.CpuIndex = cpuIndex;
    req.Gdtr = *pGdtr;

    DWORD dwRetBytes = 0;
    DeviceIoControl(m_hDriver, CTL_GET_GDT_DATA, &req, sizeof(GDT_DATA_REQ),
        pBuffer, gdtSize, &dwRetBytes, NULL);

    if (pRetBytes) *pRetBytes = dwRetBytes;
    return pBuffer;
}

std::vector<GDT_INFO> ArkR3::GDTGetVec()
{  
    SendVA(GetKernelSymbolVA("NtClose"));
    SendVA(GetKernelSymbolVA("PsActiveProcessHead"));
    GDTVec_.clear();

    
    SYSTEM_INFO SystemInfo;
    GetSystemInfo(&SystemInfo);
    Log("GetGDTVec GDT dwNumberOfProcessors:%d\n", SystemInfo.dwNumberOfProcessors);

    for (UINT i = 0; i < SystemInfo.dwNumberOfProcessors; i++) { 
       // Log("GetGDTVec CPU %d: GDTR Base=%p, Limit=%X\n", i, (void*)gdtr.Base, gdtr.Limit);
        GDTR gdtr = { 0 };
        DWORD dwRetBytes = 0;
        //这里R3拿的GDTR是错误的 但是我不想破坏代码结构，所以传个空的
        PSEGDESC pGdtData = GDTGetSingle(i, &gdtr, &dwRetBytes);
        if (pGdtData) {
            DWORD descCount = dwRetBytes / 8; 
            Log("GetGDTVec CPU %d: 解析 %d 个段描述符 (总共%d字节)\n", i, descCount, dwRetBytes);

            for (USHORT index = 0; index < descCount; index++) {
                PSEGDESC pDesc = (PSEGDESC)((PUCHAR)pGdtData + index * 8);
                
                // 解析成GDT_INFO
                GDT_INFO gdtInfo = { 0 };
                gdtInfo.cpuIndex = i;
                gdtInfo.selector = index * 8;
                
                // 声明段类型描述变量
                const char* segmentType = "Unknown";
                
                // 64位基址重组：Base1(16) + Base2(8) + Base3(8)
                gdtInfo.base = pDesc->Base1 | 
                               ((ULONG64)pDesc->Base2 << 16) | 
                               ((ULONG64)pDesc->Base3 << 24);
                
                // 检查是否为64位系统段描述符（TSS/LDT）
                if (pDesc->s == 0 && (pDesc->type == 2 || pDesc->type == 9 || pDesc->type == 11)) {
                    // 读取下一个8字节获取扩展基址
                    if ((unsigned)index + 1 < descCount) {
                        PULONG pExtended = (PULONG)((PUCHAR)pGdtData + (index + 1) * 8);
                        gdtInfo.base |= ((ULONG64)pExtended[0] << 32);  // Base4
                        gdtInfo.is_system_64 = TRUE;
                        
                        Log("合并64位系统段: 0x%04X (跳过0x%04X)\n", index * 8, (index + 1) * 8);
                        index++; // 跳过下一个条目
                    }
                }
                
                gdtInfo.limit = pDesc->Limit1 | (pDesc->Limit2 << 16);
                gdtInfo.g = pDesc->g;
                gdtInfo.l = pDesc->l;
                gdtInfo.dpl = pDesc->dpl;
                gdtInfo.type = pDesc->type;
                gdtInfo.system = pDesc->s;
                gdtInfo.p = pDesc->p;
                
                // 64位段类型解析
                if (gdtInfo.system == 0) {
                    switch (gdtInfo.type) {
                    case 9: segmentType = "64-bit TSS (Available)"; break;
                    case 11: segmentType = "64-bit TSS (Busy)"; break;
                    case 12: segmentType = "64-bit Call Gate"; break;
                    case 14: segmentType = "64-bit Interrupt Gate"; break;
                    case 15: segmentType = "64-bit Trap Gate"; break;
                    default: segmentType = " "; break;
                    }
                } else {
                    if (gdtInfo.type & 8) {  // 代码段
                        if (gdtInfo.l) {
                            segmentType = "64-bit Code";
                        } else {
                            segmentType = (gdtInfo.type & 2) ? "32-bit Code (R E)" : "32-bit Code (E)";
                        }
                    } else {  // 数据段
                        segmentType = "Data Segment";
                    }
                }
                
                strcpy_s(gdtInfo.typeDesc, sizeof(gdtInfo.typeDesc), segmentType);
                
                GDTVec_.emplace_back(gdtInfo);
            }

            free(pGdtData);

        }
        else {
            Log("GetGDTVec CPU %d: pGdtData nullptr\n", i);
        }

    }

    Log("GetGDTVec成功获取 %zu 个段描述符信息\n", GDTVec_.size());
    return GDTVec_;
}


DWORD ArkR3::ProcessGetCount()
{
    DWORD dwBytes;
    DWORD dwEntryNum = NULL;

    DeviceIoControl(m_hDriver, CTL_ENUM_PROCESS_COUNT, NULL, NULL, &dwEntryNum, sizeof(DWORD), &dwBytes, NULL);

    return dwEntryNum;
}


std::vector<PROCESS_INFO> ArkR3::ProcessGetVec(DWORD processCount)
{
    DWORD dwRetBytes;
    DWORD dwBufferSize = sizeof(PROCESS_INFO) * processCount;
    PPROCESS_INFO pEntryInfo = (PPROCESS_INFO)malloc(dwBufferSize);
    BOOL bResult = DeviceIoControl(m_hDriver, CTL_ENUM_PROCESS, NULL, NULL, pEntryInfo, dwBufferSize, &dwRetBytes, NULL);

    ProcVec_.clear();

    DWORD Count = 0;
    if (bResult) {
        Count = dwRetBytes / sizeof(PROCESS_INFO);
        for (DWORD i = 0; i < Count; i++) {
            PROCESS_INFO pInfo = pEntryInfo[i];
            ProcVec_.emplace_back(pInfo);

            Log("ProcessGetVec 进程[%d]: PID=%d, 父PID=%d, 名称=%s, EPROCESS=%p\n",
                i, pInfo.ProcessId, pInfo.ParentProcessId,
                pInfo.ImageFileName, pInfo.EprocessAddr);
        }
    }

    free(pEntryInfo);

    return ProcVec_;
}

//// 获取进程信息并存储到数组中  
//std::vector<PROCESSENTRY32> ArkR3::EnumProcesses32() {
//    HANDLE hSnapshot = INVALID_HANDLE_VALUE;
//    PROCESSENTRY32 pe32;
//
//    ProcVec_.clear();
//
//    // 创建进程快照
//    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
//    if (hSnapshot == INVALID_HANDLE_VALUE) {
//        LogErr("CreateToolhelp32Snapshot失败\n");
//        return ProcVec_;
//    }
//
//    pe32.dwSize = sizeof(PROCESSENTRY32);
//
//    if (!Process32First(hSnapshot, &pe32)) {
//        Log("Process32First失败\n");
//        CloseHandle(hSnapshot);
//        return ProcVec_;
//    }
//
//    do {
//        ProcVec_.push_back(pe32);
//    } while (Process32Next(hSnapshot, &pe32));
//
//    CloseHandle(hSnapshot);
//    Log("获取到 %d 个进程\n", (int)ProcVec_.size());
//    return ProcVec_;
//}

//读取
//R3 : [PROCESS_MEM_REQ] → R0 → R0 : [读取数据] → R3
//发送12字节请求          接收Size字节
//BOOL ArkR3::MemAttachRead(DWORD ProcessId, ULONG VirtualAddress, DWORD Size)
//{
//    // 参数验证
//    if (ProcessId == 0 || Size == 0) {
//        LogErr("AttachReadMem: Invalid args");
//        return FALSE;
//    }
//
//    // 确保内部缓冲区足够大
//    if (!MemEnsureBufferSize(Size)) {
//        LogErr("AttachReadMem: Failed to ensure buffer size");
//        return FALSE;
//    }
//
//    // 构造请求结构体（栈上分配即可）
//    PROCESS_MEM_REQ req;
//    req.ProcessId = (HANDLE)ProcessId;
//    req.VirtualAddress = (PVOID)VirtualAddress;
//    req.Size = Size;
//
//    DWORD dwRetBytes = 0;
//    BOOL bResult = DeviceIoControl(m_hDriver, CTL_ATTACH_MEM_READ,
//        &req, sizeof(PROCESS_MEM_REQ),           // 输入：请求结构体
//        memBuffer_, Size,                        // 输出：直接写入内部缓冲区
//        &dwRetBytes, NULL);
//
//    if (bResult) {
//        memDataSize_ = Size;  // 无需额外拷贝！
//
//        Log("AttachReadMem: PID=%d, Addr=0x%08X, Size=%d - Success",
//            ProcessId, VirtualAddress, Size);
//
//        return TRUE;
//    }
//    else {
//        Log("AttachReadMem: DeviceIoControl failed, PID=%d, Addr=0x%08X, Size=%d",
//            ProcessId, VirtualAddress, Size);
//        return FALSE;
//    }
//}

//写入：
//R3 : [PROCESS_MEM_REQ] [写入数据] → R0 → 处理完成
//发送12 + Size字节              不需要返回数据
//BOOL ArkR3::MemAttachWrite(DWORD ProcessId, ULONG VirtualAddress, DWORD Size)
//{
//    // 参数验证
//    if (ProcessId == 0 || VirtualAddress == 0 || Size == 0) {
//        Log("AttachWriteMem: Invalid parameters");
//        return FALSE;
//    }
//
//    // 检查内部缓冲区是否有足够的数据
//    if (memDataSize_ < Size) {
//        Log("AttachWriteMem: Not enough data in buffer, available: %d, required: %d",
//            memDataSize_, Size);
//        return FALSE;
//    }
//
//    DWORD totalSize = sizeof(PROCESS_MEM_REQ) + Size;
//    PVOID pBuffer = malloc(totalSize);
//
//    if (!pBuffer) {
//        Log("AttachWriteMem: Failed to allocate buffer, size: %d", totalSize);
//        return FALSE;
//    }
//
//    // 构造请求头
//    PPROCESS_MEM_REQ req = (PPROCESS_MEM_REQ)pBuffer;
//    req->ProcessId = (HANDLE)ProcessId;
//    req->VirtualAddress = (PVOID)VirtualAddress;
//    req->Size = Size;
//
//    // 从内部缓冲区复制数据到请求缓冲区
//    memcpy((PUCHAR)pBuffer + sizeof(PROCESS_MEM_REQ), memBuffer_, Size);
//
//    DWORD dwRetBytes = 0;
//    BOOL bResult = DeviceIoControl(m_hDriver, CTL_ATTACH_MEM_WRITE,
//        pBuffer, totalSize,
//        NULL, 0,
//        &dwRetBytes, NULL);
//
//    if (bResult) {
//        Log("AttachWriteMem: PID=%d, Addr=0x%08X, Size=%d - Success",
//            ProcessId, VirtualAddress, Size);
//    }
//    else {
//        Log("AttachWriteMem: PID=%d, Addr=0x%08X, Size=%d (dwRetBytes=%d)",
//            ProcessId, VirtualAddress, Size, dwRetBytes);
//    }
//
//    free(pBuffer);
//    return bResult;
//}

// 获取内核模块数量
DWORD ArkR3::ModuleGetCount()
{
    DWORD dwBytes = 0;
    DWORD dwEntryNum = 0;

    DeviceIoControl(m_hDriver, CTL_ENUM_MODULE_COUNT, NULL, NULL, &dwEntryNum, sizeof(DWORD), &dwBytes, NULL);

    return dwEntryNum;
}

//获取内核模块信息
std::vector<MODULE_INFO> ArkR3::ModuleGetVec(DWORD moduleCount)
{
    DWORD dwRetBytes;
    DWORD dwBufferSize = sizeof(MODULE_INFO) * moduleCount;
    PMODULE_INFO pEntryInfo = (PMODULE_INFO)malloc(dwBufferSize);
    BOOL bResult = DeviceIoControl(m_hDriver, CTL_ENUM_MODULE, NULL, NULL, pEntryInfo, dwBufferSize, &dwRetBytes, NULL);

    MoudleVec_.clear();

    DWORD Count = 0;
    if (bResult) {
        Count = dwRetBytes / sizeof(MODULE_INFO);
        for (DWORD i = 0; i < Count; i++) {
            MODULE_INFO mInfo = pEntryInfo[i];

            // 处理完整路径
            std::string fullPath = mInfo.FullPath;
            if (fullPath.find("\\SystemRoot\\") == 0) {
                fullPath = "C:\\Windows\\" + fullPath.substr(12);
            }
            else if (fullPath.find("\\WINDOWS\\") == 0) {
                fullPath = "C:\\Windows\\" + fullPath.substr(9);
            }
            else if (fullPath.find("\\??\\C:") == 0) {
                fullPath = "C:" + fullPath.substr(6);
            }
            strcpy_s(mInfo.FullPath, sizeof(mInfo.FullPath), fullPath.c_str());

            MoudleVec_.emplace_back(mInfo);

            Log("ModuleGetVec 模块[%d]: 名称=%s, 基地址=%p, 大小=0x%X, 路径=%s\n",
                i, mInfo.Name, mInfo.ImageBase, mInfo.ImageSize, mInfo.FullPath);
        }
    }

    free(pEntryInfo);
    return MoudleVec_;
}

//// 获取指定进程的模块数量
//DWORD ArkR3::ProcessModuleGetCount(DWORD processId)
//{
//    DWORD dwBytes = 0;
//    DWORD dwEntryNum = 0;
//
//    PROCESS_MODULE_REQ req;
//    req.ProcessId = (HANDLE)processId;
//    req.ModuleCount = 0;
//
//    DeviceIoControl(m_hDriver, CTL_ENUM_PROCESS_MODULE_COUNT, &req, sizeof(req),
//        &dwEntryNum, sizeof(DWORD), &dwBytes, NULL);
//
//    return dwEntryNum;
//}

//// 获取指定进程的模块信息
//std::vector<MODULE_INFO> ArkR3::ProcessModuleGetVec(DWORD processId, DWORD moduleCount)
//{
//    DWORD dwRetBytes;
//    DWORD dwBufferSize = sizeof(MODULE_INFO) * moduleCount;
//    PMODULE_INFO pEntryInfo = (PMODULE_INFO)malloc(dwBufferSize);
//
//    PROCESS_MODULE_REQ req;
//    req.ProcessId = (HANDLE)processId;
//    req.ModuleCount = moduleCount;
//
//    // 将请求结构复制到缓冲区开头
//    memcpy(pEntryInfo, &req, sizeof(req));
//
//    BOOL bResult = DeviceIoControl(m_hDriver, CTL_ENUM_PROCESS_MODULE,
//        pEntryInfo, dwBufferSize,
//        pEntryInfo, dwBufferSize, &dwRetBytes, NULL);
//
//    ProcessModuleVec_.clear();
//
//    DWORD Count = 0;
//    if (bResult) {
//        Count = dwRetBytes / sizeof(MODULE_INFO);
//        for (DWORD i = 0; i < Count; i++) {
//            MODULE_INFO mInfo = pEntryInfo[i];
//            ProcessModuleVec_.emplace_back(mInfo);
//
//            Log("ProcessModuleGetVec 进程[%d]模块[%d]: 名称=%s, 基地址=%p, 大小=0x%X\n",
//                processId, i, mInfo.Name, mInfo.ImageBase, mInfo.ImageSize);
//        }
//    }
//
//    free(pEntryInfo);
//
//    return ProcessModuleVec_;
//}

// 获取SSDT信息
std::vector<SSDT_INFO> ArkR3::SSDTGetVec()
{
    SSDTVec_.clear();

    // 假设最大700个SSDT项
    int  nMaxCount = 700;
    DWORD bufferSize = sizeof(SSDT_INFO) * nMaxCount;
    PSSDT_INFO pSsdtInfo = (PSSDT_INFO)malloc(bufferSize);

    if (!pSsdtInfo) {
        Log("SSDTGetVec: malloc err\n");
        return SSDTVec_;
    }

    DWORD dwRetBytes = 0;
    BOOL bResult = DeviceIoControl(
        m_hDriver,
        CTL_ENUM_SSDT,
        NULL, 0,
        pSsdtInfo, bufferSize,
        &dwRetBytes,
        NULL
    );

    if (bResult && dwRetBytes > 0) {
        DWORD count = dwRetBytes / sizeof(SSDT_INFO);
        Log("SSDTGetVec: 成功获取%d个SSDT项\n", count);


        for (DWORD i = 0; i < count; i++) {
            
            SSDTVec_.emplace_back(pSsdtInfo[i]);

            Log("SSDT[%03d]: %s -> 0x%p\n",
                pSsdtInfo[i].Index,
                pSsdtInfo[i].FunctionName,
                pSsdtInfo[i].FunctionAddress);
        }
    }
    else {
        LogErr("SSDTGetVec: DeviceIoControl err\n");
    }

    free(pSsdtInfo);
    return SSDTVec_;
}

//BOOL ArkR3::StartSSDTHook(HOOK_SSDT_Index flag)
//{
//    PVOID sharedMemAddr = NULL;
//
//    // 发送监控标志给驱动
//    DWORD dwRetBytes = 0;
//    BOOL bResult = DeviceIoControl(m_hDriver, CTL_START_SSDTHOOK,
//        &flag, sizeof(HOOK_SSDT_Index),
//        &sharedMemAddr, sizeof(PVOID),  //返回R3共享内存地址 保存到pSharedLogBuffer_
//        &dwRetBytes, NULL);
//
//    const char* functime = g_SSDT_XP_SP3_Table[flag].FunctionName;
//
//    if (!bResult) {
//        Log("StartSSDTHook失败 函数%s: \n", functime);
//        return FALSE;
//    }
//
//    if (sharedMemAddr) {
//        Log("StartSSDTHook: 收到共享内存地址=%p\n", sharedMemAddr);
//
//        // 验证地址是否可访问
//        __try {
//            pSharedLogBuffer_ = (PLOG_BUFFER)sharedMemAddr;
//
//            // 尝试读取第一个字段来验证访问权限
//            ULONG testRead = pSharedLogBuffer_->WriteIndex;
//            Log("StartSSDTHook: 地址验证成功，WriteIndex=%d\n", testRead);
//
//        }
//        __except (EXCEPTION_EXECUTE_HANDLER) {
//            Log("StartSSDTHook: 共享内存地址不可访问: %p\n", sharedMemAddr);
//            pSharedLogBuffer_ = NULL;
//            return FALSE;
//        }
//    }
//    else {
//        Log("StartSSDTHook: 未收到有效的共享内存地址\n");
//        return FALSE;
//    }
//
//    // 创建日志读取线程
//    isHookThreadRunning_ = true;
//    if (!hLogThread_) {
//        hLogThread_ = CreateThread(NULL, 0, LogThreadProc, this, 0, NULL);
//        if (!hLogThread_) {
//            LogErr("创建日志线程失败");
//            pSharedLogBuffer_ = NULL;
//            return FALSE;
//        }
//    }
//
//    Log("StartSSDTHook 函数:%s 日志共享内存地址=%p \n", functime, pSharedLogBuffer_);
//    return TRUE;
//}

//BOOL ArkR3::EndSSDTHook(HOOK_SSDT_Index flag)
//{
//    // 停止线程
//    isHookThreadRunning_ = false;
//    if (hLogThread_) {
//        WaitForSingleObject(hLogThread_, 3000);  // 等待3秒
//        CloseHandle(hLogThread_);
//        hLogThread_ = NULL;
//    }
//
//    // 发送停止监控标志给驱动
//    DWORD dwRetBytes = 0;
//    BOOL bResult = DeviceIoControl(m_hDriver, CTL_END_SSDTHOOK,
//        &flag, sizeof(HOOK_SSDT_Index),
//        NULL, 0,
//        &dwRetBytes, NULL);
//
//    const char* funcname = g_SSDT_XP_SP3_Table[flag].FunctionName;
//
//    if (!bResult) {
//        Log("EndSSDTHook失败 函数:%s\n ", funcname);
//        return FALSE;
//    }
//
//    //这里只是关闭了一个函数的hook 不用关闭线程
//
//    Log("EndSSDTHook 函数:%s \n", funcname);
//    return TRUE;
//}

void ArkR3::ReadProcessEvents()
{
    if (!pSharedLogBuffer_) {
        Log("!pSharedLogBuffer_\n");
        return;
    }

    // 读取当前的写入和读取索引
    ULONG currentWriteIndex = pSharedLogBuffer_->WriteIndex;
    ULONG currentReadIndex = pSharedLogBuffer_->ReadIndex;

    // 如果没有新数据，直接返回
    if (currentReadIndex == currentWriteIndex) {
        return;
    }

    // 先收集所有事件到临时vector
    std::vector<PROCESS_EVENT> tempEvents;

    // 读取所有可用的事件
    while (currentReadIndex != currentWriteIndex) {
        PROCESS_EVENT event = pSharedLogBuffer_->Logs[currentReadIndex];
        tempEvents.emplace_back(event);

        // 移动到下一个读取位置
        currentReadIndex = (currentReadIndex + 1) % 1000;
    }

    // 添加到全局数组时加锁
    if (!tempEvents.empty()) {
        // EnterCriticalSection(&g_ProcessEventCS);
        for (const auto& event : tempEvents) {
          /*  g_ProcessEvents.push_back(event);*/
        }
        // LeaveCriticalSection(&g_ProcessEventCS);
    }
    // 更新读取索引
    pSharedLogBuffer_->ReadIndex = currentReadIndex;

    // 减少日志数量
    InterlockedExchangeAdd((LONG*)&pSharedLogBuffer_->LogCount, -(LONG)ProcessEventsVec_.size());


    ULONG logCount = pSharedLogBuffer_->LogCount;
    Log("ReadProcessEvents: 更新后 ReadIndex=%d, LogCount=%d\n",
        currentReadIndex, logCount);
    return;
}

DWORD __stdcall ArkR3::LogThreadProc(LPVOID lpParam)
{
    ArkR3* pThis = (ArkR3*)lpParam;
    pThis->Log("日志读取线程启动\n");

    while (1) {
        pThis->ReadProcessEvents();

        Sleep(1000);
    }

    pThis->Log("日志读取线程结束\n");

    return 0;
}

void ArkR3::SendPdbInfo() {
    if (!ntos_pdb_) return;
//SendVA(GetKernelSymbolVA("ZwCreateThread"));
//SendVA(GetKernelSymbolVA("PsActiveProcessHead"));

}

// 回调相关接口实现
std::vector<CALLBACK_INFO> ArkR3::CallbackGetVec(CALLBACK_TYPE type) {
    CallbackVec_.clear();
    
    // 假设最多300个回调
    const ULONG maxCallbacks = 300;
    const ULONG bufferSize = maxCallbacks * sizeof(CALLBACK_INFO) + sizeof(ULONG);
    PVOID buffer = malloc(bufferSize);
    if (!buffer) {
        Log("CallbackGetVec: malloc err");
        return CallbackVec_;
    }
    
    // 在缓冲区开头写回调类型
    *(PULONG)buffer = (ULONG)type;
    
    DWORD bytesRet = 0;
    BOOL result = DeviceIoControl(
        m_hDriver,
        CTL_ENUM_CALLBACK,
        buffer,
        sizeof(ULONG),                    // 输入：回调类型
        buffer,
        bufferSize,                       // 输出：回调信息数组
        &bytesRet,
        NULL
    );
    
    if (result && bytesRet > 0) {
        ULONG callbackCount = bytesRet / sizeof(CALLBACK_INFO);
        PCALLBACK_INFO callbacks = (PCALLBACK_INFO)buffer;

        for (ULONG i = 0; i < callbackCount; i++) {
            CallbackVec_.emplace_back(callbacks[i]);
        }
        
        Log("CallbackGetVec: 获取 %d 个回调 (类型=%d)", callbackCount, type);
    } else {
        Log("CallbackGetVec: 失败，错误码=%d, 返回字节=%d", GetLastError(), bytesRet);
    }
    
    free(buffer);
    
    return CallbackVec_;
}

BOOL ArkR3::CallbackDelete(CALLBACK_TYPE type, ULONG index) {

    CALLBACK_DELETE_REQ request = { type,index,0 };//TODO 这里是不是要传一个地址？

    DWORD bytesReturned = 0;
    BOOL result = DeviceIoControl(
        m_hDriver,
        CTL_DELETE_CALLBACK,
        &request,
        sizeof(request),
        NULL,
        0,
        &bytesReturned,
        NULL
    );
    
    if (result) {
        Log("CallbackDelete: 成功删除回调 (类型=%d, 索引=%d)", type, index);
    } else {
        Log("CallbackDelete: 删除回调失败 (类型=%d, 索引=%d), 错误=%d", type, index, GetLastError());
    }
    
    return result;
}
