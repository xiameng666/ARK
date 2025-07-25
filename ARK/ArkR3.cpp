#include "ArkR3.h"


// extern SSDT_INFO g_SSDT_XP_SP3_Table[];

// ���캯��
ArkR3::ArkR3() : memBuffer_(nullptr), memBufferSize_(0), memDataSize_(0)
{
    // ��ʼ��ʱ����һ��������С�Ļ�����
    MemEnsureBufferSize(4096); // ��ʼ4KB
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

// ��ezpdb��ȡPDB·�������ø�����
bool ArkR3::SetPdbPathFromEzpdb() {
    if (!ntos_pdb_) {
        Log("ezpdb����δ��ʼ��\n");
        return false;
    }

    // ��ezpdb��ȡ��ǰPDB·��
    std::string pdbPathA = ntos_pdb_->get_current_pdb_path();
    if (pdbPathA.empty()) {
        Log("�޷���ezpdb��ȡPDB·��\n");
        return false;
    }

    // ת��ΪUnicode��·����ȫӢ�ģ�
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
    // ��ȡntoskrnl.exe·��
    char* sysroot = nullptr;
    size_t len = 0;
    if (_dupenv_s(&sysroot, &len, "systemroot") != 0 || !sysroot) return false;
    ntos_path_ = std::string(sysroot) + "\\System32\\ntoskrnl.exe";
    free(sysroot);
    // ��ȡntoskrnl.exe��ַ
    ntbase_ = GetModuleBase("ntoskrnl.exe");
    if (!ntbase_) return false;

    // ��������ʼ��PDB����
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
        Log("SendVA: ����VA=0x%p\n", va);
        return TRUE;
    }
    else {
        LogErr("SendVA: ����ʧ��\n");
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
            Log("[symbol]��ȡ%s��ַ 0x%p\n", moduleName, p->ImageBase);
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
    Log("[symbol] %s ��VA: 0x%p\n", symbolName, (void*)va);
    return va;
}

ULONG ArkR3::GetKernelSymbolOffset(const char* structName, const wchar_t* fieldName)
{
    if (!ntos_pdb_)
        return 0;

    int offset = ntos_pdb_->get_attribute_offset(structName, fieldName);
    if (offset < 0)
        return 0;

    Log("[symbol] %s.%ws ��ƫ��: 0x%x\n", structName, fieldName, offset);
    return (ULONG)offset;
}

// ȷ����������С�㹻
BOOL ArkR3::MemEnsureBufferSize(DWORD requiredSize)
{
    if (requiredSize > 0x100000) { // �������1MB
        Log("EnsureBufferSize: Size too large (%d bytes)\n", requiredSize);
        return FALSE;
    }

    if (memBufferSize_ >= requiredSize) {
        return TRUE; // ��ǰ���������㹻
    }

    // �����µĻ�������С������ȡ����4KB�߽磩
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
    DWORD gdtSize = 0x1000; // �̶��󻺳���
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
        //����R3�õ�GDTR�Ǵ���� �����Ҳ����ƻ�����ṹ�����Դ����յ�
        PSEGDESC pGdtData = GDTGetSingle(i, &gdtr, &dwRetBytes);
        if (pGdtData) {
            DWORD descCount = dwRetBytes / 8; 
            Log("GetGDTVec CPU %d: ���� %d ���������� (�ܹ�%d�ֽ�)\n", i, descCount, dwRetBytes);

            for (USHORT index = 0; index < descCount; index++) {
                PSEGDESC pDesc = (PSEGDESC)((PUCHAR)pGdtData + index * 8);
                
                // ������GDT_INFO
                GDT_INFO gdtInfo = { 0 };
                gdtInfo.cpuIndex = i;
                gdtInfo.selector = index * 8;
                
                // ������������������
                const char* segmentType = "Unknown";
                
                // 64λ��ַ���飺Base1(16) + Base2(8) + Base3(8)
                gdtInfo.base = pDesc->Base1 | 
                               ((ULONG64)pDesc->Base2 << 16) | 
                               ((ULONG64)pDesc->Base3 << 24);
                
                // ����Ƿ�Ϊ64λϵͳ����������TSS/LDT��
                if (pDesc->s == 0 && (pDesc->type == 2 || pDesc->type == 9 || pDesc->type == 11)) {
                    // ��ȡ��һ��8�ֽڻ�ȡ��չ��ַ
                    if ((unsigned)index + 1 < descCount) {
                        PULONG pExtended = (PULONG)((PUCHAR)pGdtData + (index + 1) * 8);
                        gdtInfo.base |= ((ULONG64)pExtended[0] << 32);  // Base4
                        gdtInfo.is_system_64 = TRUE;
                        
                        Log("�ϲ�64λϵͳ��: 0x%04X (����0x%04X)\n", index * 8, (index + 1) * 8);
                        index++; // ������һ����Ŀ
                    }
                }
                
                gdtInfo.limit = pDesc->Limit1 | (pDesc->Limit2 << 16);
                gdtInfo.g = pDesc->g;
                gdtInfo.l = pDesc->l;
                gdtInfo.dpl = pDesc->dpl;
                gdtInfo.type = pDesc->type;
                gdtInfo.system = pDesc->s;
                gdtInfo.p = pDesc->p;
                
                // 64λ�����ͽ���
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
                    if (gdtInfo.type & 8) {  // �����
                        if (gdtInfo.l) {
                            segmentType = "64-bit Code";
                        } else {
                            segmentType = (gdtInfo.type & 2) ? "32-bit Code (R E)" : "32-bit Code (E)";
                        }
                    } else {  // ���ݶ�
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

    Log("GetGDTVec�ɹ���ȡ %zu ������������Ϣ\n", GDTVec_.size());
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

            Log("ProcessGetVec ����[%d]: PID=%d, ��PID=%d, ����=%s, EPROCESS=%p\n",
                i, pInfo.ProcessId, pInfo.ParentProcessId,
                pInfo.ImageFileName, pInfo.EprocessAddr);
        }
    }

    free(pEntryInfo);

    return ProcVec_;
}

//// ��ȡ������Ϣ���洢��������  
//std::vector<PROCESSENTRY32> ArkR3::EnumProcesses32() {
//    HANDLE hSnapshot = INVALID_HANDLE_VALUE;
//    PROCESSENTRY32 pe32;
//
//    ProcVec_.clear();
//
//    // �������̿���
//    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
//    if (hSnapshot == INVALID_HANDLE_VALUE) {
//        LogErr("CreateToolhelp32Snapshotʧ��\n");
//        return ProcVec_;
//    }
//
//    pe32.dwSize = sizeof(PROCESSENTRY32);
//
//    if (!Process32First(hSnapshot, &pe32)) {
//        Log("Process32Firstʧ��\n");
//        CloseHandle(hSnapshot);
//        return ProcVec_;
//    }
//
//    do {
//        ProcVec_.push_back(pe32);
//    } while (Process32Next(hSnapshot, &pe32));
//
//    CloseHandle(hSnapshot);
//    Log("��ȡ�� %d ������\n", (int)ProcVec_.size());
//    return ProcVec_;
//}

//��ȡ
//R3 : [PROCESS_MEM_REQ] �� R0 �� R0 : [��ȡ����] �� R3
//����12�ֽ�����          ����Size�ֽ�
//BOOL ArkR3::MemAttachRead(DWORD ProcessId, ULONG VirtualAddress, DWORD Size)
//{
//    // ������֤
//    if (ProcessId == 0 || Size == 0) {
//        LogErr("AttachReadMem: Invalid args");
//        return FALSE;
//    }
//
//    // ȷ���ڲ��������㹻��
//    if (!MemEnsureBufferSize(Size)) {
//        LogErr("AttachReadMem: Failed to ensure buffer size");
//        return FALSE;
//    }
//
//    // ��������ṹ�壨ջ�Ϸ��伴�ɣ�
//    PROCESS_MEM_REQ req;
//    req.ProcessId = (HANDLE)ProcessId;
//    req.VirtualAddress = (PVOID)VirtualAddress;
//    req.Size = Size;
//
//    DWORD dwRetBytes = 0;
//    BOOL bResult = DeviceIoControl(m_hDriver, CTL_ATTACH_MEM_READ,
//        &req, sizeof(PROCESS_MEM_REQ),           // ���룺����ṹ��
//        memBuffer_, Size,                        // �����ֱ��д���ڲ�������
//        &dwRetBytes, NULL);
//
//    if (bResult) {
//        memDataSize_ = Size;  // ������⿽����
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

//д�룺
//R3 : [PROCESS_MEM_REQ] [д������] �� R0 �� �������
//����12 + Size�ֽ�              ����Ҫ��������
//BOOL ArkR3::MemAttachWrite(DWORD ProcessId, ULONG VirtualAddress, DWORD Size)
//{
//    // ������֤
//    if (ProcessId == 0 || VirtualAddress == 0 || Size == 0) {
//        Log("AttachWriteMem: Invalid parameters");
//        return FALSE;
//    }
//
//    // ����ڲ��������Ƿ����㹻������
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
//    // ��������ͷ
//    PPROCESS_MEM_REQ req = (PPROCESS_MEM_REQ)pBuffer;
//    req->ProcessId = (HANDLE)ProcessId;
//    req->VirtualAddress = (PVOID)VirtualAddress;
//    req->Size = Size;
//
//    // ���ڲ��������������ݵ����󻺳���
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

// ��ȡ�ں�ģ������
DWORD ArkR3::ModuleGetCount()
{
    DWORD dwBytes = 0;
    DWORD dwEntryNum = 0;

    DeviceIoControl(m_hDriver, CTL_ENUM_MODULE_COUNT, NULL, NULL, &dwEntryNum, sizeof(DWORD), &dwBytes, NULL);

    return dwEntryNum;
}

//��ȡ�ں�ģ����Ϣ
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

            // ��������·��
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

            Log("ModuleGetVec ģ��[%d]: ����=%s, ����ַ=%p, ��С=0x%X, ·��=%s\n",
                i, mInfo.Name, mInfo.ImageBase, mInfo.ImageSize, mInfo.FullPath);
        }
    }

    free(pEntryInfo);
    return MoudleVec_;
}

//// ��ȡָ�����̵�ģ������
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

//// ��ȡָ�����̵�ģ����Ϣ
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
//    // ������ṹ���Ƶ���������ͷ
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
//            Log("ProcessModuleGetVec ����[%d]ģ��[%d]: ����=%s, ����ַ=%p, ��С=0x%X\n",
//                processId, i, mInfo.Name, mInfo.ImageBase, mInfo.ImageSize);
//        }
//    }
//
//    free(pEntryInfo);
//
//    return ProcessModuleVec_;
//}

// ��ȡSSDT��Ϣ
std::vector<SSDT_INFO> ArkR3::SSDTGetVec()
{
    SSDTVec_.clear();

    // �������700��SSDT��
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
        Log("SSDTGetVec: �ɹ���ȡ%d��SSDT��\n", count);


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
//    // ���ͼ�ر�־������
//    DWORD dwRetBytes = 0;
//    BOOL bResult = DeviceIoControl(m_hDriver, CTL_START_SSDTHOOK,
//        &flag, sizeof(HOOK_SSDT_Index),
//        &sharedMemAddr, sizeof(PVOID),  //����R3�����ڴ��ַ ���浽pSharedLogBuffer_
//        &dwRetBytes, NULL);
//
//    const char* functime = g_SSDT_XP_SP3_Table[flag].FunctionName;
//
//    if (!bResult) {
//        Log("StartSSDTHookʧ�� ����%s: \n", functime);
//        return FALSE;
//    }
//
//    if (sharedMemAddr) {
//        Log("StartSSDTHook: �յ������ڴ��ַ=%p\n", sharedMemAddr);
//
//        // ��֤��ַ�Ƿ�ɷ���
//        __try {
//            pSharedLogBuffer_ = (PLOG_BUFFER)sharedMemAddr;
//
//            // ���Զ�ȡ��һ���ֶ�����֤����Ȩ��
//            ULONG testRead = pSharedLogBuffer_->WriteIndex;
//            Log("StartSSDTHook: ��ַ��֤�ɹ���WriteIndex=%d\n", testRead);
//
//        }
//        __except (EXCEPTION_EXECUTE_HANDLER) {
//            Log("StartSSDTHook: �����ڴ��ַ���ɷ���: %p\n", sharedMemAddr);
//            pSharedLogBuffer_ = NULL;
//            return FALSE;
//        }
//    }
//    else {
//        Log("StartSSDTHook: δ�յ���Ч�Ĺ����ڴ��ַ\n");
//        return FALSE;
//    }
//
//    // ������־��ȡ�߳�
//    isHookThreadRunning_ = true;
//    if (!hLogThread_) {
//        hLogThread_ = CreateThread(NULL, 0, LogThreadProc, this, 0, NULL);
//        if (!hLogThread_) {
//            LogErr("������־�߳�ʧ��");
//            pSharedLogBuffer_ = NULL;
//            return FALSE;
//        }
//    }
//
//    Log("StartSSDTHook ����:%s ��־�����ڴ��ַ=%p \n", functime, pSharedLogBuffer_);
//    return TRUE;
//}

//BOOL ArkR3::EndSSDTHook(HOOK_SSDT_Index flag)
//{
//    // ֹͣ�߳�
//    isHookThreadRunning_ = false;
//    if (hLogThread_) {
//        WaitForSingleObject(hLogThread_, 3000);  // �ȴ�3��
//        CloseHandle(hLogThread_);
//        hLogThread_ = NULL;
//    }
//
//    // ����ֹͣ��ر�־������
//    DWORD dwRetBytes = 0;
//    BOOL bResult = DeviceIoControl(m_hDriver, CTL_END_SSDTHOOK,
//        &flag, sizeof(HOOK_SSDT_Index),
//        NULL, 0,
//        &dwRetBytes, NULL);
//
//    const char* funcname = g_SSDT_XP_SP3_Table[flag].FunctionName;
//
//    if (!bResult) {
//        Log("EndSSDTHookʧ�� ����:%s\n ", funcname);
//        return FALSE;
//    }
//
//    //����ֻ�ǹر���һ��������hook ���ùر��߳�
//
//    Log("EndSSDTHook ����:%s \n", funcname);
//    return TRUE;
//}

void ArkR3::ReadProcessEvents()
{
    if (!pSharedLogBuffer_) {
        Log("!pSharedLogBuffer_\n");
        return;
    }

    // ��ȡ��ǰ��д��Ͷ�ȡ����
    ULONG currentWriteIndex = pSharedLogBuffer_->WriteIndex;
    ULONG currentReadIndex = pSharedLogBuffer_->ReadIndex;

    // ���û�������ݣ�ֱ�ӷ���
    if (currentReadIndex == currentWriteIndex) {
        return;
    }

    // ���ռ������¼�����ʱvector
    std::vector<PROCESS_EVENT> tempEvents;

    // ��ȡ���п��õ��¼�
    while (currentReadIndex != currentWriteIndex) {
        PROCESS_EVENT event = pSharedLogBuffer_->Logs[currentReadIndex];
        tempEvents.emplace_back(event);

        // �ƶ�����һ����ȡλ��
        currentReadIndex = (currentReadIndex + 1) % 1000;
    }

    // ��ӵ�ȫ������ʱ����
    if (!tempEvents.empty()) {
        // EnterCriticalSection(&g_ProcessEventCS);
        for (const auto& event : tempEvents) {
          /*  g_ProcessEvents.push_back(event);*/
        }
        // LeaveCriticalSection(&g_ProcessEventCS);
    }
    // ���¶�ȡ����
    pSharedLogBuffer_->ReadIndex = currentReadIndex;

    // ������־����
    InterlockedExchangeAdd((LONG*)&pSharedLogBuffer_->LogCount, -(LONG)ProcessEventsVec_.size());


    ULONG logCount = pSharedLogBuffer_->LogCount;
    Log("ReadProcessEvents: ���º� ReadIndex=%d, LogCount=%d\n",
        currentReadIndex, logCount);
    return;
}

DWORD __stdcall ArkR3::LogThreadProc(LPVOID lpParam)
{
    ArkR3* pThis = (ArkR3*)lpParam;
    pThis->Log("��־��ȡ�߳�����\n");

    while (1) {
        pThis->ReadProcessEvents();

        Sleep(1000);
    }

    pThis->Log("��־��ȡ�߳̽���\n");

    return 0;
}

void ArkR3::SendPdbInfo() {
    if (!ntos_pdb_) return;
//SendVA(GetKernelSymbolVA("ZwCreateThread"));
//SendVA(GetKernelSymbolVA("PsActiveProcessHead"));

}

// �ص���ؽӿ�ʵ��
std::vector<CALLBACK_INFO> ArkR3::CallbackGetVec(CALLBACK_TYPE type) {
    CallbackVec_.clear();
    
    // �������300���ص�
    const ULONG maxCallbacks = 300;
    const ULONG bufferSize = maxCallbacks * sizeof(CALLBACK_INFO) + sizeof(ULONG);
    PVOID buffer = malloc(bufferSize);
    if (!buffer) {
        Log("CallbackGetVec: malloc err");
        return CallbackVec_;
    }
    
    // �ڻ�������ͷд�ص�����
    *(PULONG)buffer = (ULONG)type;
    
    DWORD bytesRet = 0;
    BOOL result = DeviceIoControl(
        m_hDriver,
        CTL_ENUM_CALLBACK,
        buffer,
        sizeof(ULONG),                    // ���룺�ص�����
        buffer,
        bufferSize,                       // ������ص���Ϣ����
        &bytesRet,
        NULL
    );
    
    if (result && bytesRet > 0) {
        ULONG callbackCount = bytesRet / sizeof(CALLBACK_INFO);
        PCALLBACK_INFO callbacks = (PCALLBACK_INFO)buffer;

        for (ULONG i = 0; i < callbackCount; i++) {
            CallbackVec_.emplace_back(callbacks[i]);
        }
        
        Log("CallbackGetVec: ��ȡ %d ���ص� (����=%d)", callbackCount, type);
    } else {
        Log("CallbackGetVec: ʧ�ܣ�������=%d, �����ֽ�=%d", GetLastError(), bytesRet);
    }
    
    free(buffer);
    
    return CallbackVec_;
}

BOOL ArkR3::CallbackDelete(CALLBACK_TYPE type, ULONG index) {

    CALLBACK_DELETE_REQ request = { type,index,0 };//TODO �����ǲ���Ҫ��һ����ַ��

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
        Log("CallbackDelete: �ɹ�ɾ���ص� (����=%d, ����=%d)", type, index);
    } else {
        Log("CallbackDelete: ɾ���ص�ʧ�� (����=%d, ����=%d), ����=%d", type, index, GetLastError());
    }
    
    return result;
}
