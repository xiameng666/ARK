#define _CRT_SECURE_NO_WARNINGS
#include "pestruct.h"
#define IDC_DYNAMIC_BUTTON 4000


PEparser::PEparser(const char* filePath)
    : m_hFile(INVALID_HANDLE_VALUE),
    m_hMapping(NULL),
    m_PEStruct{ 0 },
    m_pFileBase(nullptr),
    m_fileSize(0)
{
    OpenFile(filePath);
}

IMAGE_DOS_HEADER& PEparser::GetDosHeader() { return m_PEStruct.DOS_HEADER.header; }
IMAGE_NT_HEADERS& PEparser::GetNtHeader() { return m_PEStruct.NT_HEADERS.header; }
IMAGE_FILE_HEADER& PEparser::GetFileHeader() { return m_PEStruct.NT_HEADERS.header.FileHeader; }
IMAGE_OPTIONAL_HEADER& PEparser::GetOptHeader() { return m_PEStruct.NT_HEADERS.header.OptionalHeader; }
IMAGE_SECTION_HEADER& PEparser::GetSectionHeader(int index) { return m_PEStruct.SECTION_HEADERS[index].headers; }
DWORD PEparser::GetSectionTableOffset() {
    return m_PEStruct.NT_HEADERS.offset +
        sizeof(DWORD) +
        sizeof(IMAGE_FILE_HEADER) +
        GetFileHeader().SizeOfOptionalHeader;
}
DWORD PEparser::GetDosHeaderOffset() {
    return m_PEStruct.DOS_HEADER.offset;
}
DWORD PEparser::GetNtHeaderOffset() {
    return m_PEStruct.NT_HEADERS.offset;
}
DWORD PEparser::GetFileHeaderOffset() {
    return m_PEStruct.NT_HEADERS.offset + sizeof(DWORD);
}
DWORD PEparser::GetOptionalHeaderOffset() {
    return GetFileHeaderOffset() + sizeof(IMAGE_FILE_HEADER);
}
DWORD PEparser::GetSectionHeaderOffset(int index) {
    return GetSectionTableOffset() + index * sizeof(IMAGE_SECTION_HEADER);
}
DWORD PEparser::AlignUp(DWORD value, DWORD alignment) {
    return (value + alignment - 1) & ~(alignment - 1);
}
DWORD PEparser::AlignToFile(DWORD value)
{
    return AlignUp(value, GetOptHeader().FileAlignment);
}
DWORD PEparser::AlignToSection(DWORD value)
{
    return AlignUp(value, GetOptHeader().SectionAlignment);
}

PEparser::~PEparser()
{
    Clean();

    if (m_PEStruct.SECTION_HEADERS) {
        delete[] m_PEStruct.SECTION_HEADERS;
        m_PEStruct.SECTION_HEADERS = nullptr;
    }
}

void PEparser::Clean()
{
    if (m_hMapping != NULL) {
        CloseHandle(m_hMapping);
        m_hMapping = NULL;
    }

    if (m_pFileBase != NULL) {
        UnmapViewOfFile(m_pFileBase);
        m_pFileBase = NULL;
    }

    if (m_hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(m_hFile);
        m_hFile = INVALID_HANDLE_VALUE;
    }
}

void PEparser::OpenAgain()
{
    Clean();
    Sleep(3000);
    OpenFile(m_filePath);
}

void PEparser::OpenFile(const char* filePath)
{
    m_filePath = filePath;//�����ļ�·��
    m_hFile = CreateFile(filePath, GENERIC_READ, FILE_SHARE_READ,
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (m_hFile == INVALID_HANDLE_VALUE)
    {
        char errorMsg[256];
        sprintf_s(errorMsg, "OpenFile err: %d", GetLastError());
        throw std::runtime_error(errorMsg);
    }

    m_hMapping = CreateFileMapping(m_hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if (!m_hMapping)
    {
        char errorMsg[256];
        sprintf_s(errorMsg, "CreateFileMapping err: %d", GetLastError());
        throw std::runtime_error(errorMsg);
    }

    m_pFileBase = MapViewOfFile(m_hMapping, FILE_MAP_READ, 0, 0, 0);
    if (!m_pFileBase) {
        CloseHandle(m_hMapping);
        CloseHandle(m_hFile);
        char errorMsg[256];
        sprintf_s(errorMsg, "MapViewOfFile err: %d", GetLastError());
        throw std::runtime_error(errorMsg);
    }

    m_fileSize = ::GetFileSize(m_hFile, NULL);
    if (m_fileSize == INVALID_FILE_SIZE) {
        char errorMsg[256];
        sprintf_s(errorMsg, "GetFileSize err: %d", GetLastError());
        throw std::runtime_error(errorMsg);
    }
};

void PEparser::ParseDos()
{
    ZeroMemory(&m_PEStruct.DOS_HEADER, sizeof(MY_IMAGE_DOS_HEADER));
    memcpy(&m_PEStruct.DOS_HEADER.header, m_pFileBase, sizeof(IMAGE_DOS_HEADER));

    m_PEStruct.DOS_HEADER.offset = 0;
    if (m_PEStruct.DOS_HEADER.header.e_magic != IMAGE_DOS_SIGNATURE)
    {
        throw std::runtime_error("IMAGE_DOS_SIGNATURE err");
    }
}

void PEparser::ParseNTHeaders()
{
    ZeroMemory(&m_PEStruct.NT_HEADERS, sizeof(MY_IMAGE_NT_HEADERS));

    //e_lfanew����ntͷ�����ļ���ƫ��
    m_PEStruct.NT_HEADERS.offset = m_PEStruct.DOS_HEADER.header.e_lfanew;
    memcpy(&m_PEStruct.NT_HEADERS.header,
        (BYTE*)m_pFileBase + m_PEStruct.NT_HEADERS.offset, sizeof(IMAGE_NT_HEADERS32));

    if (m_PEStruct.NT_HEADERS.header.Signature != IMAGE_NT_SIGNATURE)
    {
        throw std::runtime_error("IMAGE_NT_SIGNATURE err");
    }
}

void PEparser::ParseSection()
{
    if (m_PEStruct.SECTION_HEADERS)
    {
        delete[] m_PEStruct.SECTION_HEADERS;
        m_PEStruct.SECTION_HEADERS = nullptr;
    }

    //��ȡ�ڱ������������ڴ�
    WORD NumberOfSections = GetFileHeader().NumberOfSections;

    if (NumberOfSections == 0)
    {
        MessageBox(NULL, "�ڱ�����Ϊ0", "ParseSection", MB_OK);
    }

    m_PEStruct.SECTION_HEADERS = new MY_IMAGE_SECTION_HEADER[NumberOfSections];

    //sectionoffset  = ntͷ+sign+fileheader+optheader
    DWORD sectionOffset
        = m_PEStruct.NT_HEADERS.offset
        + sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER)
        + GetFileHeader().SizeOfOptionalHeader;

    //��ȡ���нڱ�
    for (WORD i = 0;i < NumberOfSections;i++)
    {
        //��ǰ�ڱ����ļ���ƫ��
        DWORD curOffset = sectionOffset + i * sizeof(IMAGE_SECTION_HEADER);

        ZeroMemory(&m_PEStruct.SECTION_HEADERS[i], sizeof(MY_IMAGE_SECTION_HEADER));
        m_PEStruct.SECTION_HEADERS[i].offset = curOffset;
        memcpy(&m_PEStruct.SECTION_HEADERS[i].headers,
            (BYTE*)m_pFileBase + curOffset,
            sizeof(IMAGE_SECTION_HEADER));
    }
}

BOOL PEparser::ParseImportDir()
{
    m_PEStruct.ImportInfo.ImportModuleVec.clear();
    m_PEStruct.ImportInfo.AllImportFunctions.clear();

    // ��ȡ����Ŀ¼λ��
    IMAGE_DATA_DIRECTORY ImportTableDir = GetOptHeader().DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    if (ImportTableDir.VirtualAddress == 0) {
        return FALSE;
    }

    // ת��Ϊ�ļ�ƫ��
    DWORD dwImportFOA = RVAToFOA(ImportTableDir.VirtualAddress);
    if (dwImportFOA == 0) {
        return FALSE;
    }

    BYTE* fileBase = (BYTE*)m_pFileBase;

    // ���������
    IMAGE_IMPORT_DESCRIPTOR* pImportTable = (IMAGE_IMPORT_DESCRIPTOR*)(fileBase + dwImportFOA);

    while (pImportTable->Name != 0)
    {
        ImportModuleInfo moduleInfo = { 0 };

        // ���ƻ�����Ϣ
        moduleInfo.OriginalFirstThunk = pImportTable->OriginalFirstThunk;
        moduleInfo.TimeDateStamp = pImportTable->TimeDateStamp;
        moduleInfo.ForwarderChain = pImportTable->ForwarderChain;
        moduleInfo.NameRVA = pImportTable->Name;
        moduleInfo.FirstThunk = pImportTable->FirstThunk;

        // ��ȡDLL����
        DWORD nameFOA = RVAToFOA(pImportTable->Name);
        if (nameFOA == 0 || nameFOA >= m_fileSize - 1) break;

        const char* dllname = (char*)(fileBase + nameFOA);
        strncpy_s(moduleInfo.ModuleName, dllname, MAX_PATH - 1);

        //TODO
        // ���������б�
        DWORD thunkRVA = (pImportTable->OriginalFirstThunk != 0) ?
            pImportTable->OriginalFirstThunk : pImportTable->FirstThunk;
        DWORD thunkFOA = RVAToFOA(thunkRVA);
        DWORD iatRVA = pImportTable->FirstThunk;
        DWORD iatFOA = RVAToFOA(iatRVA);

        if (thunkFOA != 0 && iatFOA != 0)
        {
            IMAGE_THUNK_DATA32* pThunk = (IMAGE_THUNK_DATA32*)(fileBase + thunkFOA);
            IMAGE_THUNK_DATA32* pIAT = (IMAGE_THUNK_DATA32*)(fileBase + iatFOA);

            while (pThunk->u1.AddressOfData != 0)
            {
                ImportFunctionInfo funcInfo = { 0 };

                // ����ģ������
                strncpy_s(funcInfo.ModuleName, moduleInfo.ModuleName, MAX_PATH - 1);

                // ����OFTs��IATֵ
                funcInfo.OriginalThunkValue = pThunk->u1.Function;
                funcInfo.FirstThunkValue = pIAT->u1.Function;

                // �ж��Ƿ���ŵ���
                if (pThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG32) {
                    funcInfo.bOrdinal = TRUE;
                    funcInfo.Ordinal = IMAGE_ORDINAL32(pThunk->u1.Ordinal);
                    strcpy_s(funcInfo.Name, "(Ordinal)");
                }
                else {
                    // �����Ƶ���
                    DWORD importByNameFOA = RVAToFOA(pThunk->u1.AddressOfData);
                    if (importByNameFOA != 0) {
                        IMAGE_IMPORT_BY_NAME* pImportByName =
                            (IMAGE_IMPORT_BY_NAME*)(fileBase + importByNameFOA);
                        funcInfo.Hint = pImportByName->Hint;
                        strncpy_s(funcInfo.Name, (char*)pImportByName->Name, MAX_PATH - 1);
                    }
                }

                // ��ӵ������б�
                m_PEStruct.ImportInfo.AllImportFunctions.push_back(funcInfo);

                pThunk++;
                pIAT++;
                moduleInfo.FunctionCount++;
            }
        }

        m_PEStruct.ImportInfo.ImportModuleVec.push_back(moduleInfo);
        pImportTable++;
    }

    return !m_PEStruct.ImportInfo.ImportModuleVec.empty();
}

BOOL PEparser::ParseExportDir()
{
    m_PEStruct.ExportInfo = { 0 };
    IMAGE_DATA_DIRECTORY ExportTableDir = GetOptHeader().DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
    if (ExportTableDir.VirtualAddress == 0 ) //size����
    {
        return FALSE;// �޵�����
    }

    // ��ȡ������
    DWORD dwExportFOA = RVAToFOA(ExportTableDir.VirtualAddress);
    BYTE* fileBase  = (BYTE*)m_pFileBase;
    IMAGE_EXPORT_DIRECTORY* pExportDir = (IMAGE_EXPORT_DIRECTORY*)(fileBase + dwExportFOA);
    memcpy(&m_PEStruct.ExportInfo.Directory, pExportDir, sizeof(IMAGE_EXPORT_DIRECTORY));

    //������ַ�� �������Ʊ� ��ű�
    DWORD* pAddressOfFunctions = NULL;
    DWORD* pAddressOfNames = NULL;
    WORD* pAddressOfNameOrdinals = NULL;

    if (pExportDir->AddressOfFunctions)
    {
        DWORD funcTableFOA = RVAToFOA(pExportDir->AddressOfFunctions);
        pAddressOfFunctions = (DWORD*)(fileBase + funcTableFOA);
    }

    if (pExportDir->AddressOfNames) 
    {
        DWORD nameTableFOA = RVAToFOA(pExportDir->AddressOfNames);
        pAddressOfNames = (DWORD*)(fileBase + nameTableFOA);
    }

    if (pExportDir->AddressOfNameOrdinals) 
    {
        DWORD ordTableFOA = RVAToFOA(pExportDir->AddressOfNameOrdinals);
        pAddressOfNameOrdinals = (WORD*)(fileBase + ordTableFOA);
    }

    if (!pAddressOfFunctions || pExportDir->NumberOfFunctions == 0) 
    {
        printf("!pAddressOfFunctions || pExportDir->NumberOfFunctions == 0");
        return FALSE;
    }

    for (DWORD i = 0; i < pExportDir->NumberOfFunctions; i++)
    {
        if (pAddressOfFunctions[i] == 0) continue;

        //�ں�����ַ������ź�RVA
        ExportFuncInfo funcInfo = { 0 };
        funcInfo.Ordinal = pExportDir->Base + i;
        funcInfo.FunctionRVA = pAddressOfFunctions[i];

        // �ں������Ʊ���Һ�����Ӧ����
        if (pAddressOfNames != NULL && pAddressOfNameOrdinals != NULL)
        {
            for (DWORD j = 0; j < pExportDir->NumberOfNames; j++)
            {
                if (pAddressOfNameOrdinals[j] == i)
                {
                    // �ҵ�������
                    funcInfo.NameOrdinal = (WORD)i;
                    funcInfo.NameRVA = pAddressOfNames[j];
                    DWORD nameFOA = RVAToFOA(pAddressOfNames[j]);
                    strncpy_s(funcInfo.Name, (char*)(fileBase + nameFOA), MAX_PATH - 1);
                    break;
                }

            }
        }
        

        // ����Ƿ�Ϊת������
        if (pAddressOfFunctions[i] >= ExportTableDir.VirtualAddress &&
            pAddressOfFunctions[i] < ExportTableDir.VirtualAddress + ExportTableDir.Size) {

            DWORD forwardFOA = RVAToFOA(pAddressOfFunctions[i]);
            char forwardStr[MAX_PATH] = { 0 };
            strncpy_s(forwardStr, (char*)(fileBase + forwardFOA), MAX_PATH - 1);

            // ��ת����Ϣ��ӵ���������
            char fullName[MAX_PATH] = { 0 };
            sprintf_s(fullName, "[Forward to %s] %s", forwardStr, funcInfo.Name);
            strncpy_s(funcInfo.Name, fullName, MAX_PATH - 1);
        }

        m_PEStruct.ExportInfo.Functions.push_back(funcInfo);
    }

    return TRUE;
}

BOOL PEparser::ParseTLSDir()
{
    ZeroMemory(&m_PEStruct.tlsInfo.Dir, sizeof(IMAGE_TLS_DIRECTORY));
    m_PEStruct.tlsInfo.CallbacksVec.clear();

    // ��ȡTLSĿ¼
    IMAGE_DATA_DIRECTORY tlsDir = GetOptHeader().DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS];
    if (tlsDir.VirtualAddress == 0) {
        return FALSE; // ��TLSĿ¼
    }

    DWORD tlsFOA = RVAToFOA(tlsDir.VirtualAddress);

    // ��ȡTLSĿ¼
    IMAGE_TLS_DIRECTORY32* pTlsDir = (IMAGE_TLS_DIRECTORY32*)((BYTE*)m_pFileBase + tlsFOA);
    memcpy(&m_PEStruct.tlsInfo.Dir, pTlsDir, sizeof(IMAGE_TLS_DIRECTORY));

    // �����ص�������
    if (pTlsDir->AddressOfCallBacks)
    {
        DWORD callbackTableRVA = VAToRVA(pTlsDir->AddressOfCallBacks);
        DWORD callbackTableFOA = RVAToFOA(callbackTableRVA);

        DWORD* callbacks = (DWORD*)((BYTE*)m_pFileBase + callbackTableFOA);

        for (size_t i = 0;callbacks[i] != 0;i++)
        {
            m_PEStruct.tlsInfo.CallbacksVec.push_back(callbacks[i]);
        }
    }

    return TRUE;
}

/*

BOOL PEparser::RebuildIat(const char* filePath)
{
    PEparser targetPE(filePath);
    targetPE.Parse();

    std::map<std::string, std::vector<IAT_ENTRY>> dllFunctions;
    for (const auto& entry : m_IatEntries) {
        if (strlen(entry.dllName) > 0 && strcmp(entry.dllName, "<δ֪>") != 0 &&
            strcmp(entry.dllName, "<δ֪ģ��>") != 0) {
            dllFunctions[entry.dllName].push_back(entry);
        }
    }
    // ��������ռ��С
    DWORD sizeNeeded = 0;
    // ����Ŀ¼���С
    sizeNeeded += sizeof(IMAGE_IMPORT_DESCRIPTOR) * (dllFunctions.size() + 1);
    return 0;
}
*/
DWORD PEparser::FOAToRVA(DWORD fileoffset)
{
    // ��鸺��
    if (fileoffset > 0x80000000)
        return 0;
        
    if (fileoffset < m_PEStruct.NT_HEADERS.header.OptionalHeader.SizeOfHeaders)
        return fileoffset;

    WORD numberOfSections = GetFileHeader().NumberOfSections;
    if (numberOfSections == 0) return 0;

    for (WORD i = 0; i < numberOfSections; i++)
    {
        IMAGE_SECTION_HEADER& section = m_PEStruct.SECTION_HEADERS[i].headers;

        if (fileoffset >= section.PointerToRawData &&
            fileoffset < section.PointerToRawData + section.SizeOfRawData)
        {
            //���ڵ�ƫ��
            DWORD offset = fileoffset - section.PointerToRawData;

            return offset + section.VirtualAddress;
        }

    }
    return 0;
}

DWORD PEparser::RVAToFOA(DWORD rva)
{
    // ��鸺��
    if (rva > 0x80000000)
        return 0;
        
    //ͷ RVA=FILEOFFSET
    if (rva < GetOptHeader().SizeOfHeaders)
        return rva;

    WORD numberOfSections = GetFileHeader().NumberOfSections;
    if (numberOfSections == 0) return 0;

    for (WORD i = 0; i < numberOfSections; i++)
    {
        IMAGE_SECTION_HEADER& section = m_PEStruct.SECTION_HEADERS[i].headers;

        if (rva >= section.VirtualAddress &&
            rva < section.VirtualAddress + section.Misc.VirtualSize)
        {
        
            // �����Ƿ����ļ�����
            if (section.PointerToRawData == 0 || section.SizeOfRawData == 0)
            {
                return 0;  
            }

            //�ڽ��ڵ�ƫ��
            DWORD offset = rva - section.VirtualAddress;
            return offset + section.PointerToRawData;
        }

    }

    return 0;
}

DWORD PEparser::RVAToVA(DWORD rva)
{
    // ��鸺��
    if (rva > 0x80000000)
        return 0;
        
    return rva + m_PEStruct.NT_HEADERS.header.OptionalHeader.ImageBase;
}

DWORD PEparser::VAToRVA(DWORD va)
{
    // ��鸺��
    if (va > 0x80000000)
        return 0;
        
    // ���VA�Ƿ�С��ImageBase
    if (va < m_PEStruct.NT_HEADERS.header.OptionalHeader.ImageBase)
        return 0;
        
    return va - m_PEStruct.NT_HEADERS.header.OptionalHeader.ImageBase;
}
