
#include "WinsockHeader.h"

#include "ArkR3.h"

PEparser* pe_ = nullptr;

// extern SSDT_INFO g_SSDT_XP_SP3_Table[];

// 构造函数
ArkR3::ArkR3() : memBuffer_(nullptr), memBufferSize_(0), memDataSize_(0)
{
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

    if (pe_!=nullptr) free (pe_);
}

// 从ezpdb获取PDB路径并发给驱动
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
    // === 初始化ntoskrnl.exe符号 ===
    char* sysroot = nullptr;
    size_t len = 0;
    if (_dupenv_s(&sysroot, &len, "systemroot") != 0 || !sysroot) return false;
    ntos_path_ = std::string(sysroot) + "\\System32\\ntoskrnl.exe";
    //free(sysroot);

    // 获取ntoskrnl.exe基址
    ntbase_ = GetModuleBase("ntoskrnl.exe");
    if (!ntbase_) return false;

    // 创建并初始化PDB对象
    ntos_pdb_ = std::make_unique<ez::pdb>(ntos_path_, "http://msdl.blackint3.com:88/download/symbols/");
    if (!ntos_pdb_->init()) {
        ntos_pdb_.reset();
        return false;
    }

    // === 初始化win32k.sys符号 ===
    win32k_path_ = std::string(sysroot) + "\\System32\\win32k.sys";
    free(sysroot);

    // 获取win32k.sys基址
    win32k_base_ = GetModuleBase("win32k.sys");
    if (!win32k_base_) {
        Log("Win32k.sys not loaded \n");
        // 不是错误，服务进程可能没有加载win32k
    }

    // 创建并初始化win32k PDB对象
    win32k_pdb_ = std::make_unique<ez::pdb>(win32k_path_, "http://msdl.blackint3.com:88/download/symbols/");
    if (!win32k_pdb_->init()) {
        Log("Failed to initialize win32k PDB\n");
        win32k_pdb_.reset();
    }
    else {
        Log("Win32k PDB init success\n");
    }

    return true;
}


ULONG_PTR ArkR3::GetSSDTBaseRVA() {
    ULONG_PTR ssdtRVA = 0;
    ULONG bytesReturned = 0;

    BOOL result = DeviceIoControl(
        m_hDriver,
        CTL_SEND_SSDTBASE,
        NULL, 0,
        &ssdtRVA, sizeof(ULONG_PTR),
        &bytesReturned,
        NULL
    );

    if (result) {
        Log("[R3] 成功获取KiServiceTable RVA: 0x%x\n", ssdtRVA);
        return ssdtRVA;
    }

    Log("[R3] 获取KiServiceTable RVA失败 - result:%d, bytes:%d",
        result, bytesReturned);
    return 0;
}

/*
void ArkR3::GetFileSSDT() {
    pe_ = new PEparser(ntos_path_.c_str());
    pe_->Parse();

    ULONG_PTR dwRVA = GetSSDTBaseRVA();
    DWORD ssdtFOA = pe_->RVAToFOA(dwRVA);

    HMODULE hMod = LoadLibrary("ntoskrnl.exe");
    DWORD dwVA = (DWORD)GetProcAddress(hMod, "KeServiceDescriptorTable");
    DWORD dwR3RVA = dwVA - ntbase_;
    Log("R0获取的RVA:%p  转化为FOA %p  R3获取的RVA %p\n", dwRVA, ssdtFOA, dwR3RVA);

    PVOID fileBase = pe_->m_pFileBase;
    PSYSTEM_SERVICE_DESCRIPTOR_TABLE pFileSSDT =
        (PSYSTEM_SERVICE_DESCRIPTOR_TABLE)((ULONG_PTR)fileBase + ssdtFOA);

    Log("文件中的SSDT结构:\n");
    Log("  Base: %p", pFileSSDT->Base);
    Log("  NumberOfServices: %d\n", pFileSSDT->NumberOfServices);
    Log("  ServiceCounterTable: %p\n", pFileSSDT->ServiceCounterTable);
    Log("  ParamTableBase: %p\n", pFileSSDT->ParamTableBase);
}
*/
bool ArkR3::RestoreSSdt()
{
    DWORD bytesReturned = 0;

    // 调用驱动执行SSDT恢复
    BOOL result = DeviceIoControl(
        m_hDriver,
        CTL_RESTORE_SSDT,
        nullptr,               
        0,                     
        nullptr,               
        0,                     
        &bytesReturned,
        nullptr
    );

    if (result) {
        Log("SSDT恢复成功\n");
        return true;
    }
    else {
        LogErr("SSDT恢复失败\n");
        return false;
    }
}

BOOL ArkR3::SendVA(ULONG_PTR va)
{
    if (m_hDriver == INVALID_HANDLE_VALUE) {
        Log("SendVA: err\n");
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

DWORD ArkR3::IdtGetCount() {
    ULONG idtCount = 0;
    DWORD dwRetBytes = 0;

    BOOL bResult = DeviceIoControl(
        m_hDriver,
        CTL_ENUM_IDT_COUNT,
        NULL, 0,
        &idtCount, sizeof(ULONG),
        &dwRetBytes,
        NULL
    );

    if (!bResult || idtCount == 0) {
        LogErr("IdtGetVec: 获取IDT数量失败\n");
        return 0;
    }

    Log("IdtGetVec: 发现 %d 个IDT条目\n", idtCount);

    return idtCount;
}

std::vector<IDT_INFO> ArkR3::IdtGetVec()
{
    IDTVec_.clear();

    ULONG bufferSize = IdtGetCount() * sizeof(IDT_INFO);
    PIDT_INFO pIdtData = (PIDT_INFO)malloc(bufferSize);
    if (!pIdtData) {
        LogErr("IdtGetVec: malloc 失败\n");
        return IDTVec_;
    }

    DWORD dwRetBytes = 0;
    BOOL bResult = DeviceIoControl(
        m_hDriver,
        CTL_ENUM_IDT,
        NULL, 0,
        pIdtData, bufferSize,
        &dwRetBytes,
        NULL
    );

    if (bResult && dwRetBytes > 0) {
        ULONG actualCount = dwRetBytes / sizeof(IDT_INFO);
        Log("IdtGetVec: 成功获取 %d 个IDT条目\n", actualCount);

        // 直接添加到vector中
        for (ULONG i = 0; i < actualCount; i++) {
            

            /**/
            if (strstr(pIdtData->Path, "ntoskrnl.exe") != NULL) {
                int rva = pIdtData[i].Address - ntbase_;
                std::string funcName = ntos_pdb_->get_function_name(rva);
                strcpy_s(pIdtData[i].funcName, sizeof(pIdtData[i].funcName),
                    funcName.empty() ? "Unknown" : funcName.c_str());
            }

            //修复模块路径
            std::string fixedPath = FixModulePath(pIdtData[i].Path);
            strcpy_s(pIdtData[i].Path, sizeof(pIdtData[i].Path), fixedPath.c_str());

            IDTVec_.emplace_back(pIdtData[i]);

            /*
            Log("IDT[%d]: CPU%d INT%02X FuncName=%s Sel=0x%04X DPL=%d Addr=0x%p Path=%s\n",
                i, pIdtData[i].CpuId, pIdtData[i].id, pIdtData[i].funcName,
                pIdtData[i].Selector, pIdtData[i].Dpl,
                (PVOID)pIdtData[i].Address, pIdtData[i].Path);
            */
        }
    }
    else {
        LogErr("IdtGetVec: 获取IDT数据失败\n");
    }

    free(pIdtData);
    return IDTVec_;
}


BOOL ArkR3::ProcessForceKill(ULONG ProcessId)
{
    DWORD bytesReturned = 0;
    BOOL result = DeviceIoControl(
        m_hDriver,
        CTL_FORCE_KILL_PROCESS,
        &ProcessId,
        sizeof(ULONG),
        nullptr,
        0,
        &bytesReturned,
        nullptr
    );

    if (!result) {
        LogErr("ProcessForceKill 失败 ");
        return false;
    }

    return true;
}

DWORD ArkR3::ProcessGetCount()
{
    DWORD dwBytes;
    DWORD dwEntryNum = NULL;

    DeviceIoControl(m_hDriver, CTL_ENUM_PROCESS_COUNT, NULL, NULL, &dwEntryNum, sizeof(DWORD), &dwBytes, NULL);

    return dwEntryNum;
}

std::vector<PROCESS_INFO> ArkR3::ProcessSearchGetVec()
{
    ProcSearchVec_.clear();

    DWORD processCount = 1000;//假设最大1000项
    DWORD dwRetBytes;
    DWORD dwBufferSize = sizeof(PROCESS_INFO) * processCount;
    PPROCESS_INFO pEntryInfo = (PPROCESS_INFO)malloc(dwBufferSize);
    BOOL bResult = DeviceIoControl(m_hDriver, CTL_MEMSEARCH_PROCESS, NULL, NULL, pEntryInfo, dwBufferSize, &dwRetBytes, NULL);

    DWORD Count = 0;
    if (bResult) {
        Count = dwRetBytes / sizeof(PROCESS_INFO);
        for (DWORD i = 0; i < Count; i++) {
            PROCESS_INFO pInfo = pEntryInfo[i];
            ProcSearchVec_.emplace_back(pInfo);

            
            Log("ProcessSearchGetVec 进程[%d]: PID=%d, 父PID=%d, 名称=%s, EPROCESS=%p\n",
                i, pInfo.ProcessId, pInfo.ParentProcessId,
                pInfo.ImageFileName, pInfo.EprocessAddr);
                
        }
    }

    free(pEntryInfo);

    return ProcSearchVec_;
}

std::vector<PROCESS_INFO> ArkR3::ProcessGetVec(DWORD processCount)
{
    ProcVec_.clear();

    DWORD dwRetBytes;
    DWORD dwBufferSize = sizeof(PROCESS_INFO) * processCount;
    PPROCESS_INFO pEntryInfo = (PPROCESS_INFO)malloc(dwBufferSize);
    BOOL bResult = DeviceIoControl(m_hDriver, CTL_ENUM_PROCESS, NULL, NULL, pEntryInfo, dwBufferSize, &dwRetBytes, NULL);



    DWORD Count = 0;
    if (bResult) {
        Count = dwRetBytes / sizeof(PROCESS_INFO);
        for (DWORD i = 0; i < Count; i++) {
            PROCESS_INFO pInfo = pEntryInfo[i];
            ProcVec_.emplace_back(pInfo);

            /*
            Log("ProcessGetVec 进程[%d]: PID=%d, 父PID=%d, 名称=%s, EPROCESS=%p\n",
                i, pInfo.ProcessId, pInfo.ParentProcessId,
                pInfo.ImageFileName, pInfo.EprocessAddr);
                */
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

//取
//R3 : [PROCESS_MEM_REQ] → R0 → R0 : [读取数据] → R3
//发送12字节请求          接收Size字节
BOOL ArkR3::MemAttachRead(DWORD ProcessId, ULONG_PTR VirtualAddress, DWORD Size)
{
    // 参数验证
    if (ProcessId == 0 || Size == 0) {
        LogErr("AttachReadMem: Invalid args\n");
        return FALSE;
    }

    // 确保内部缓冲区足够大
    if (!MemEnsureBufferSize(Size)) {
        LogErr("AttachReadMem: Failed to ensure buffer size\n");
        return FALSE;
    }

    // 构造请求结构体
    PROCESS_MEM_REQ req;
    req.ProcessId = (HANDLE)ProcessId;// UlongToHandle(ProcessId);
    req.VirtualAddress = (PVOID)VirtualAddress;
    req.Size = Size;

    DWORD dwRetBytes = 0;
    BOOL bResult = DeviceIoControl(m_hDriver, CTL_ATTACH_MEM_READ,
        &req, sizeof(PROCESS_MEM_REQ),           // 输入：请求结构体
        memBuffer_, Size,                        // 输出：写入memBuffer_
        &dwRetBytes, NULL);

    if (bResult) {
        memDataSize_ = Size;  

        Log("AttachReadMem: PID=%d, Addr=0x%016llX, Size=%d - Success\n",
            ProcessId, VirtualAddress, Size);

        return TRUE;
    }
    else {
        Log("AttachReadMem: DeviceIoControl failed, PID=%d, Addr=0x%016llX, Size=%d\n",
            ProcessId, VirtualAddress, Size);
        return FALSE;
    }
}

//写入：
//R3 : [PROCESS_MEM_REQ] [写入数据] → R0 → 处理完成
//发送12 + Size字节              不需要返回数据
BOOL ArkR3::MemAttachWrite(DWORD ProcessId, ULONG_PTR VirtualAddress, DWORD Size)
{
    // 参数验证
    if (ProcessId == 0 || VirtualAddress == 0 || Size == 0) {
        Log("AttachWriteMem: Invalid parameters\n");
        return FALSE;
    }

    // 检查内部缓冲区是否有足够的数据
    if (memDataSize_ < Size) {
        Log("AttachWriteMem: Not enough data in buffer, available: %d, required: %d\n",
            memDataSize_, Size);
        return FALSE;
    }

    DWORD totalSize = sizeof(PROCESS_MEM_REQ) + Size;
    PVOID pBuffer = malloc(totalSize);

    if (!pBuffer) {
        Log("AttachWriteMem: Failed to allocate buffer, size: %d\n", totalSize);
        return FALSE;
    }

    // 构造请求头
    PPROCESS_MEM_REQ req = (PPROCESS_MEM_REQ)pBuffer;
    req->ProcessId = (HANDLE)ProcessId;
    req->VirtualAddress = (PVOID)VirtualAddress;
    req->Size = Size;

    // 从内部缓冲区复制数据到请求缓冲区
    memcpy((PUCHAR)pBuffer + sizeof(PROCESS_MEM_REQ), memBuffer_, Size);

    DWORD dwRetBytes = 0;
    BOOL bResult = DeviceIoControl(m_hDriver, CTL_ATTACH_MEM_WRITE,
        pBuffer, totalSize,
        NULL, 0,
        &dwRetBytes, NULL);

    if (bResult) {
        Log("AttachWriteMem: PID=%d, Addr=0x%016llX, Size=%d - Success\n",
            ProcessId, VirtualAddress, Size);
    }
    else {
        Log("AttachWriteMem: PID=%d, Addr=0x%016llX, Size=%d (dwRetBytes=%d)\n",
            ProcessId, VirtualAddress, Size, dwRetBytes);
    }

    free(pBuffer);
    return bResult;
}

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
            std::string fixedPath = FixModulePath(mInfo.FullPath);
            strcpy_s(mInfo.FullPath, sizeof(mInfo.FullPath), fixedPath.c_str());

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

std::string ArkR3::GetWin32kFunctionName(ULONG_PTR address) {

    int rva = address - win32k_base_;
    //int rva = address;
    return win32k_pdb_->get_function_name(rva);
}

// ShadowSSDT枚举
std::vector<ShadowSSDT_INFO> ArkR3::ShadowSSDTGetVec() {
    ShadowSSDTVec_.clear();

    // 假设最大1000项
    int maxCount = 2000;
    DWORD bufferSize = sizeof(SSDT_INFO) * maxCount;
    PSSDT_INFO pShadowSsdtInfo = (PSSDT_INFO)malloc(bufferSize);

    if (!pShadowSsdtInfo) {
        Log("ShadowSSDTGetVec: malloc err\n");
        return ShadowSSDTVec_;
    }

    DWORD dwRetBytes = 0;
    BOOL bResult = DeviceIoControl(
        m_hDriver,
        CTL_ENUM_ShadowSSDT,
        NULL, 0,
        pShadowSsdtInfo, bufferSize,
        &dwRetBytes,
        NULL
    );

    if (bResult && dwRetBytes > 0) {
        DWORD count = dwRetBytes / sizeof(SSDT_INFO);
        Log("ShadowSSDTGetVec: 成功获取%d个ShadowSSDT项\n", count);

        for (DWORD i = 0; i < count; i++) {

            std::string win32kName = GetWin32kFunctionName((ULONG_PTR)pShadowSsdtInfo[i].FunctionAddress);
            if (!win32kName.empty()) {
                strcpy_s(pShadowSsdtInfo[i].FunctionName,
                    sizeof(pShadowSsdtInfo[i].FunctionName),
                    win32kName.c_str());
                Log("\"%s\"\n", win32kName.c_str());
                /*Log("ShadowSSDT[%03d]: 符号解析成功: %s -> 0x%p\n",
                    i, win32kName.c_str(), pShadowSsdtInfo[i].FunctionAddress);
                */
            }

            ShadowSSDTVec_.emplace_back(pShadowSsdtInfo[i]);
        }
    }
    else {
        LogErr("ShadowSSDTGetVec: DeviceIoControl err, 可能ShadowSSDT不可用\n");
    }

    free(pShadowSsdtInfo);
    return ShadowSSDTVec_;
}


void ArkR3::SendPdbInfo() {
    if (!ntos_pdb_) return;
//SendVA(GetKernelSymbolVA("ZwCreateThread"));
//SendVA(GetKernelSymbolVA("PsActiveProcessHead"));

}

// 回调相关
std::vector<CALLBACK_INFO> ArkR3::CallbackGetVec(CALLBACK_TYPE type) {
    CallbackVec_.clear();
    
    // 假设最多300个回调
    const ULONG maxCallbacks = 300;
    const ULONG bufferSize = maxCallbacks * sizeof(CALLBACK_INFO) + sizeof(ULONG);
    PVOID buffer = malloc(bufferSize);
    if (!buffer) {
        Log("CallbackGetVec: malloc err\n");
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
    
    if (result) {
        ULONG callbackCount = bytesRet / sizeof(CALLBACK_INFO);
        PCALLBACK_INFO callbacks = (PCALLBACK_INFO)buffer;

        for (ULONG i = 0; i < callbackCount; i++) {
            std::string fixedPath = FixModulePath(callbacks[i].ModulePath);
            strcpy_s(callbacks[i].ModulePath, sizeof(callbacks[i].ModulePath), fixedPath.c_str());

            CallbackVec_.emplace_back(callbacks[i]);
        }
        
    } else {
        Log("CallbackGetVec: 失败，错误码=%d, 返回字节=%d\n", GetLastError(), bytesRet);
    }
    
    free(buffer);
    
    return CallbackVec_;
}

std::wstring ArkR3::FixModulePath(const std::wstring& path) {
    if (path.find(L"\\SystemRoot\\") == 0) {
        return L"C:\\Windows\\" + path.substr(12);
    }
    else if (path.find(L"\\WINDOWS\\") == 0) {
        return L"C:\\Windows\\" + path.substr(9);
    }
    else if (path.find(L"\\??\\C:") == 0) {
        return L"C:" + path.substr(6);
    }
    else if (path.find(L"\\??\\") == 0) {
        return path.substr(4);
    }
    return path;
}

std::string ArkR3::FixModulePath(const std::string& path) {
    if (path.find("\\SystemRoot\\") == 0) {
        return "C:\\Windows\\" + path.substr(12);
    }
    else if (path.find("\\WINDOWS\\") == 0) {
        return "C:\\Windows\\" + path.substr(9);
    }
    else if (path.find("\\??\\C:") == 0) {
        return "C:" + path.substr(6);
    }
    else if (path.find("\\??\\") == 0) {
        return path.substr(4);
    }
    return path;
}

BOOL ArkR3::CallbackDelete(CALLBACK_TYPE type, ULONG index, PVOID CallbackFuncAddr) {

    CALLBACK_DELETE_REQ request = { type,index,CallbackFuncAddr};

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
        Log("CallbackDelete: 成功删除回调 (类型=%d, 索引=%d)\n", type, index);
    } else {
        Log("CallbackDelete: 删除回调失败 (类型=%d, 索引=%d), 错误=%d\n", type, index, GetLastError());
    }
    
    return result;
}

std::vector<DISPATCH_HOOK_INFO> ArkR3::DispatchHookGetVec() {
    DispatchHookVec_.clear();

    //分配缓冲区给1000个hook
    const DWORD maxHooks = 2000;
    DWORD bufferSize = maxHooks * sizeof(DISPATCH_HOOK_INFO);
    PDISPATCH_HOOK_INFO buffer = (PDISPATCH_HOOK_INFO)malloc(bufferSize);

    if (!buffer) {
        Log("DispatchHookGetVec: malloc error\n");
        return DispatchHookVec_;
    }

    DWORD bytesRet = 0;
    BOOL result = DeviceIoControl(
        m_hDriver,
        CTL_ENUM_DISPATCH_HOOK,
        NULL,
        0,
        buffer,
        bufferSize,
        &bytesRet,
        NULL
    );

    if (result) {
        ULONG hookCount = bytesRet / sizeof(DISPATCH_HOOK_INFO);
        Log("DispatchHookGetVec: 获取 %d 个hook派遣函数\n", hookCount);

        // Copy results to member variable
        for (ULONG i = 0; i < hookCount; i++) {
            DispatchHookVec_.emplace_back(buffer[i]);
        }

    }
    else {
        Log("DispatchHookGetVec: 失败, error=%d, bytes=%d\n", GetLastError(),
            bytesRet);
    }

    free(buffer);

    return DispatchHookVec_;
}

// Attached Device
std::vector<DEVICE_STACK_INFO> ArkR3::DeviceStackGetVec() {
    DeviceStackVec_.clear();

    // 分配缓冲区给设备栈信息
    const DWORD maxStacks = 2000;
    DWORD bufferSize = maxStacks * sizeof(DEVICE_STACK_INFO);
    PDEVICE_STACK_INFO buffer = (PDEVICE_STACK_INFO)malloc(bufferSize);

    if (!buffer) {
        Log("DeviceStackGetVec: malloc error\n");
        return DeviceStackVec_;
    }

    DWORD bytesRet = 0;
    BOOL result = DeviceIoControl(
        m_hDriver,
        CTL_ENUM_DEVICE_STACK,
        NULL,
        0,
        buffer,
        bufferSize,
        &bytesRet,
        NULL
    );

    if (result) {
        
        ULONG stackCount = bytesRet / sizeof(DEVICE_STACK_INFO);
        Log("DeviceStackGetVec: 分析了 %d 个设备栈\n", stackCount);

        std::set<PVOID> processedDrivers;//去重

        // 将结果复制到成员变量
        for (ULONG i = 0; i < stackCount; i++) {
            DEVICE_STACK_INFO stackInfo = buffer[i];

            std::wstring normalizedOrigPath =
                FixModulePath(stackInfo.OriginalDriverPath);
            wcscpy_s(stackInfo.OriginalDriverPath,
                sizeof(stackInfo.OriginalDriverPath) / sizeof(WCHAR),
                normalizedOrigPath.c_str());

            for (ULONG j = 0; j < stackInfo.FilterCount; j++) {
                std::wstring normalizedFilterPath =
                    FixModulePath(stackInfo.Filters[j].DriverPath);
                wcscpy_s(stackInfo.Filters[j].DriverPath,
                    sizeof(stackInfo.Filters[j].DriverPath) / sizeof(WCHAR),
                    normalizedFilterPath.c_str());
            }

            DeviceStackVec_.emplace_back(stackInfo);
        }

        /*
        std::set<PVOID> processedDrivers;//去重

        for (ULONG i = 0; i < stackCount; i++) {
            PVOID driverObj = buffer[i].OriginalDriverObject;

            // 如果这个驱动对象已经处理过，跳过
            if (processedDrivers.find(driverObj) != processedDrivers.end()) {
                continue;
            }

            // 记录这个驱动对象已处理
            processedDrivers.insert(driverObj);

            DeviceStackVec_.emplace_back(buffer[i]);
        }
        */

    }
    else {
        Log("DeviceStackGetVec: DeviceIoControl failed, error=%d\n", GetLastError());
    }

    free(buffer);
    return DeviceStackVec_;
}

std::vector<DRIVER_OBJECT_INFO> ArkR3::DriverObjectGetVec()
{
    DriverObjectVec_.clear();

    DWORD bufferSize = 500 * sizeof(DRIVER_OBJECT_INFO);
    PDRIVER_OBJECT_INFO buffer = (PDRIVER_OBJECT_INFO)malloc(bufferSize);

    if (!buffer) {
        Log("DeviceStackGetVec: malloc error\n");
        return DriverObjectVec_;
    }

    DWORD bytesRet = 0;
    BOOL result = DeviceIoControl(
        m_hDriver,
        CTL_ENUM_DRIVER_OBJECT,
        NULL,
        0,
        buffer,
        bufferSize,
        &bytesRet,
        NULL
    );

    ULONG count = bytesRet / sizeof(DRIVER_OBJECT_INFO);

    if (result) {
        for (ULONG i = 0; i < count; i++) {
            DriverObjectVec_.emplace_back(buffer[i]);
        }
        Log("DriverObjectGetVec: 收到 %d 个驱动对象\n", count);
    }
    else {
        LogErr("DriverObjectGetVec: err\n");
    }

    return DriverObjectVec_;
}

std::vector<DRIVER_OBJECT_INFO> ArkR3::DriverHideDetect()
{
    std::vector<DRIVER_OBJECT_INFO> hideDrivers;

    auto driverObjects = DriverObjectGetVec();

    DWORD moduleCount = ModuleGetCount();
    auto modules = ModuleGetVec(moduleCount);

    for (const auto& drvObj : driverObjects) {
        bool foundInModules = false;

        // 在模块列表中查找匹配的基址
        for (const auto& module : modules) {
            if (drvObj.DriverStart == module.ImageBase) {
                foundInModules = true;
                break;
            }
        }

        if (!foundInModules) {
            hideDrivers.emplace_back(drvObj);  // 发现隐藏驱动
            Log("发现隐藏驱动: %ws at 0x%p\n", drvObj.DriverName, drvObj.DriverStart);
        }
    }

    return hideDrivers;
}

// 处理进程路径的辅助函数
void GetProcessPath(DWORD processId, char* processPath, size_t pathSize) {
    HANDLE hProcess = NULL;
    char tempPath[MAX_PATH] = "Unknown";

    if (processId == 0) {
        strcpy_s(processPath, pathSize, "System Idle Process");
        return;
    }

    if (processId == 4) {
        strcpy_s(processPath, pathSize, "System");
        return;
    }

    // 打开进程句柄
    hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId);
    if (hProcess != NULL) {
        DWORD dwSize = MAX_PATH;

        // 获取进程完整路径
        if (QueryFullProcessImageNameA(hProcess, 0, tempPath, &dwSize)) {
            strcpy_s(processPath, pathSize, tempPath);
        }
        else
        {
            // 获取路径失败，获取模块名
            if (GetModuleBaseNameA(hProcess, NULL, tempPath, sizeof(tempPath))) {
                strcpy_s(processPath, pathSize, tempPath);
            }
            else {
                sprintf_s(processPath, pathSize, "PID:%d (Access Denied)", processId);
            }
        }

        CloseHandle(hProcess);
    }
    else {
        sprintf_s(processPath, pathSize, "PID:%d (Process Not Found)", processId);
    }
}

// TCP状态转换为字符串
void ArkR3::GetTcpStateString(DWORD dwState, char* stateStr, size_t
    stateSize) {
    switch (dwState) {
    case MIB_TCP_STATE_CLOSED:
        strcpy_s(stateStr, stateSize, "CLOSED");
        break;
    case MIB_TCP_STATE_LISTEN:
        strcpy_s(stateStr, stateSize, "LISTENING");
        break;
    case MIB_TCP_STATE_SYN_SENT:
        strcpy_s(stateStr, stateSize, "SYN_SENT");
        break;
    case MIB_TCP_STATE_SYN_RCVD:
        strcpy_s(stateStr, stateSize, "SYN_RCVD");
        break;
    case MIB_TCP_STATE_ESTAB:
        strcpy_s(stateStr, stateSize, "ESTABLISHED");
        break;
    case MIB_TCP_STATE_FIN_WAIT1:
        strcpy_s(stateStr, stateSize, "FIN_WAIT1");
        break;
    case MIB_TCP_STATE_FIN_WAIT2:
        strcpy_s(stateStr, stateSize, "FIN_WAIT2");
        break;
    case MIB_TCP_STATE_CLOSE_WAIT:
        strcpy_s(stateStr, stateSize, "CLOSE_WAIT");
        break;
    case MIB_TCP_STATE_CLOSING:
        strcpy_s(stateStr, stateSize, "CLOSING");
        break;
    case MIB_TCP_STATE_LAST_ACK:
        strcpy_s(stateStr, stateSize, "LAST_ACK");
        break;
    case MIB_TCP_STATE_TIME_WAIT:
        strcpy_s(stateStr, stateSize, "TIME_WAIT");
        break;
    case MIB_TCP_STATE_DELETE_TCB:
        strcpy_s(stateStr, stateSize, "DELETE_TCB");
        break;
    default:
        strcpy_s(stateStr, stateSize, "UNKNOWN");
        break;
    }
}

// 获取网络端口信息 - R3实现
std::vector<NETWORK_PORT_INFO> ArkR3::NetworkPortGetVec(){
    NetworkPortVec_.clear();

    PMIB_TCPTABLE_OWNER_PID pTcpTable = NULL;
    DWORD dwSize = 0;
    DWORD dwRetVal = 0;

    char szLocalAddr[128];
    char szRemoteAddr[128];
    char szProcessPath[MAX_PATH];
    struct in_addr IpAddr;


    // 第一次调用获取需要的缓冲区大小
    dwRetVal = GetExtendedTcpTable(
        NULL,                    // 缓冲区指针
        &dwSize,                 // 缓冲区大小
        TRUE,                    // 排序
        AF_INET,                 // 地址族 (IPv4)
        TCP_TABLE_OWNER_PID_ALL, // 表类型 (包含PID的所有连接)
        0                        // 保留参数
    );


    if (dwRetVal == ERROR_INSUFFICIENT_BUFFER) {
        pTcpTable = (MIB_TCPTABLE_OWNER_PID*)malloc(dwSize);
        if (pTcpTable == NULL) {
            Log("Error allocating memory\n");
            return NetworkPortVec_;
        }

        // 第二次调用获取实际数据
        dwRetVal = GetExtendedTcpTable(
            pTcpTable,
            &dwSize,
            TRUE,
            AF_INET,
            TCP_TABLE_OWNER_PID_ALL,
            0
        );
    }

    //Tcp表
    if (dwRetVal == NO_ERROR) {
        Log("\tNumber of entries: %d\n", (int)pTcpTable->dwNumEntries);

        for (DWORD i = 0; i < pTcpTable->dwNumEntries; i++) {
            NETWORK_PORT_INFO portInfo = { 0 };

            // 协议类型
            strcpy_s(portInfo.Protocol, sizeof(portInfo.Protocol),"TCP");

            // 本地IP地址和端口
            IpAddr.S_un.S_addr = (u_long)pTcpTable->table[i].dwLocalAddr;
            strcpy_s(szLocalAddr, sizeof(szLocalAddr), inet_ntoa(IpAddr));

            sprintf_s(portInfo.LocalAddress, sizeof(portInfo.LocalAddress),
                "%s:%d", szLocalAddr,
                ntohs((u_short)pTcpTable->table[i].dwLocalPort));

            //远程IP地址和端口
            IpAddr.S_un.S_addr = (u_long)pTcpTable->table[i].dwRemoteAddr;
            strcpy_s(szRemoteAddr, sizeof(szRemoteAddr), inet_ntoa(IpAddr));

            sprintf_s(portInfo.RemoteAddress,sizeof(portInfo.RemoteAddress),
                "%s:%d", szRemoteAddr,
                ntohs((u_short)pTcpTable->table[i].dwRemotePort));

            // TCP状态
            GetTcpStateString(pTcpTable->table[i].dwState,
                portInfo.State, sizeof(portInfo.State));

            // 进程ID
            portInfo.ProcessId = pTcpTable->table[i].dwOwningPid;

            // 获取进程路径
            GetProcessPath(pTcpTable->table[i].dwOwningPid, szProcessPath, sizeof(szProcessPath));

            strcpy_s(portInfo.ProcessPath,sizeof(portInfo.ProcessPath), szProcessPath);

            NetworkPortVec_.push_back(portInfo);
        }
    }
    else {
        Log("\tGetExtendedTcpTable failed with %d\n", dwRetVal);
    }

    // 获取UDP表
    PMIB_UDPTABLE_OWNER_PID pUdpTable = NULL;
    dwSize = 0;
    dwRetVal = GetExtendedUdpTable(NULL, &dwSize, TRUE, AF_INET,
        UDP_TABLE_OWNER_PID, 0);

    if (dwRetVal == ERROR_INSUFFICIENT_BUFFER) {
        pUdpTable = (MIB_UDPTABLE_OWNER_PID*)malloc(dwSize);
        if (pUdpTable != NULL) {
            dwRetVal = GetExtendedUdpTable(pUdpTable, &dwSize, TRUE,
                AF_INET, UDP_TABLE_OWNER_PID, 0);

            if (dwRetVal == NO_ERROR) {
                Log("NetworkPortGetVec: 找到 %d 个UDP端口\n",
                    (int)pUdpTable->dwNumEntries);

                for (DWORD i = 0; i < pUdpTable->dwNumEntries; i++) {
                    NETWORK_PORT_INFO portInfo = { 0 };

                    //协议类型
                    strcpy_s(portInfo.Protocol,
                        sizeof(portInfo.Protocol), "UDP");

                    // 本地IP地址和端口
                    IpAddr.S_un.S_addr =
                        (u_long)pUdpTable->table[i].dwLocalAddr;
                    strcpy_s(szLocalAddr, sizeof(szLocalAddr),
                        inet_ntoa(IpAddr));

                    sprintf_s(portInfo.LocalAddress,
                        sizeof(portInfo.LocalAddress),
                        "%s:%d", szLocalAddr,
                        ntohs((u_short)pUdpTable->table[i].dwLocalPort));

                    // UDP没有远程地址
                    strcpy_s(portInfo.RemoteAddress,
                        sizeof(portInfo.RemoteAddress), "*:*");
                    strcpy_s(portInfo.State, sizeof(portInfo.State),
                        "LISTENING");

                    // 进程ID
                    portInfo.ProcessId =
                        pUdpTable->table[i].dwOwningPid;

                    // 进程路径
                    GetProcessPath(pUdpTable->table[i].dwOwningPid,
                        szProcessPath, sizeof(szProcessPath));
                    strcpy_s(portInfo.ProcessPath,
                        sizeof(portInfo.ProcessPath), szProcessPath);

                    NetworkPortVec_.push_back(portInfo);
                }
            }
            free(pUdpTable);
        }
    }

    Log("NetworkPortGetVec:总共获取 %zu 个网络端口\n", NetworkPortVec_.size());

    return NetworkPortVec_;
  }


  //typedef struct _UNICODE_STRING {
  //    USHORT Length;
  //    USHORT MaximumLength;
  //    PWCH   Buffer;
  //} UNICODE_STRING;

  //typedef UNICODE_STRING* PUNICODE_STRING;

  //typedef struct _RTL_BUFFER {
  //    PWCHAR    Buffer;
  //    PWCHAR    StaticBuffer;
  //    SIZE_T    Size;
  //    SIZE_T    StaticSize;
  //    SIZE_T    ReservedForAllocatedSize; // for future doubling
  //    PVOID     ReservedForIMalloc; // for future pluggable growth
  //} RTL_BUFFER, * PRTL_BUFFER;

  //typedef struct _RTL_UNICODE_STRING_BUFFER {
  //    UNICODE_STRING String;
  //    RTL_BUFFER     ByteBuffer;
  //    WCHAR          MinimumStaticBufferForTerminalNul[sizeof(WCHAR)];
  //} RTL_UNICODE_STRING_BUFFER, * PRTL_UNICODE_STRING_BUFFER;

  std::wstring ConvertToDevicePath(const std::wstring& dosPath) {
      if (dosPath.length() < 3) return L"";

      WCHAR driveLetter[3] = { dosPath[0], L':', L'\0' };
      WCHAR deviceName[MAX_PATH];

      if (QueryDosDeviceW(driveLetter, deviceName, MAX_PATH)) {
          std::wstring result = deviceName;
          result += dosPath.substr(2); // 跳过 "C:"
          return result;
      }
      return L"";
  }

BOOL ArkR3::UnlockFile(const std::string& filePath)
{
    // 转换路径为宽字符
    std::wstring widePath(filePath.begin(), filePath.end());
    Log("宽字符路径: %ws\n", widePath.c_str());

    std::wstring devicePath = ConvertToDevicePath(widePath);
    Log("设备路径: %ws\n", devicePath.c_str());  // 用 %ws

    if (devicePath.empty()) {
        Log("ConvertToDevicePath 失败\n");
        return FALSE;
    }

    FILE_REQ fileReq = { 0 };
    wcscpy_s(fileReq.FilePath, devicePath.c_str());

    DWORD bytesReturned = 0;
    BOOL result = DeviceIoControl(
        m_hDriver,
        CTL_UNLOCK_FILE,
        &fileReq,
        sizeof(fileReq),
        NULL,
        0,
        &bytesReturned,
        NULL
    );

    if (result) {
        Log("UnlockFile: 成功解锁文件 %s\n", filePath.c_str());
        return TRUE;
    }
    else {
        DWORD error = GetLastError();
        Log("UnlockFile: 解锁文件失败 %s, 错误码: %d\n", filePath.c_str(), error);
        return FALSE;
    }
}

BOOL ArkR3::ForceDeleteFile(const std::string& filePath)
{
    // 转换路径为宽字符
    std::wstring widePath(filePath.begin(), filePath.end());
    Log("宽字符路径: %ws\n", widePath.c_str());

    std::wstring devicePath = ConvertToDevicePath(widePath);
    Log("设备路径: %ws\n", devicePath.c_str());  

    if (devicePath.empty()) {
        Log("ConvertToDevicePath 失败\n");
        return FALSE;
    }

    FILE_REQ fileReq = { 0 };
    wcscpy_s(fileReq.FilePath, devicePath.c_str());

    DWORD bytesReturned = 0;
    BOOL result = DeviceIoControl(
        m_hDriver,
        CTL_FORCE_DELETE_FILE,
        &fileReq,
        sizeof(fileReq),
        NULL,
        0,
        &bytesReturned,
        NULL
    );

    if (result) {
        Log("ForceDeleteFile: 成功粉碎文件 %s\n", filePath.c_str());
        return TRUE;
    }
    else {
        DWORD error = GetLastError();
        Log("ForceDeleteFile: 粉碎文件失败 %s, 错误码: %d\n", filePath.c_str(), error);
        return FALSE;
    }
}

/*
// 获取网络端口信息  改为R3实现
std::vector<NETWORK_PORT_INFO> ArkR3::NetworkPortGetVec() {
    NetworkPortVec_.clear();

    // 分配缓冲区给网络端口信息
    const DWORD maxPorts = 2000;
    DWORD bufferSize = maxPorts * sizeof(NETWORK_PORT_INFO);
    PNETWORK_PORT_INFO buffer = (PNETWORK_PORT_INFO)malloc(bufferSize);

    if (!buffer) {
        Log("NetworkPortGetVec: malloc error\n");
        return NetworkPortVec_;
    }

    DWORD bytesRet = 0;
    BOOL result = DeviceIoControl(
        m_hDriver,
        CTL_ENUM_NETWORK_PORT,
        NULL,
        0,
        buffer,
        bufferSize,
        &bytesRet,
        NULL
    );

    if (result) {
        ULONG portCount = bytesRet / sizeof(NETWORK_PORT_INFO);
        Log("NetworkPortGetVec: 分析了 %d 个网络端口\n", portCount);

        // 将结果复制到成员变量
        for (ULONG i = 0; i < portCount; i++) {
            NetworkPortVec_.push_back(buffer[i]);
        }

    }
    else {
        Log("NetworkPortGetVec: DeviceIoControl failed, error=%d\n", GetLastError());
    }

    free(buffer);
    return NetworkPortVec_;
}
*/
