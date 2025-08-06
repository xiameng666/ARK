
#include <stdexcept>
#include <map>
#include <vector>

#include "winternl.h"

#define IDC_EDIT_INPLACE                     2000

#ifndef _WINDOWS_
#include<windows.h>
#endif
#include <atlstr.h>

 class PEparser
{
public:
    struct MY_IMAGE_DOS_HEADER
    {
        IMAGE_DOS_HEADER header;
        DWORD offset;
    };

    struct MY_IMAGE_NT_HEADERS
    {
        IMAGE_NT_HEADERS header;
        DWORD offset;
    };

    struct MY_IMAGE_SECTION_HEADER
    {
        IMAGE_SECTION_HEADER headers;
        DWORD offset;
    };

    struct ImportModuleInfo
    {
        DWORD OriginalFirstThunk;
        DWORD TimeDateStamp;
        DWORD ForwarderChain;
        DWORD NameRVA;
        DWORD FirstThunk;
        char ModuleName[MAX_PATH];
        int FunctionCount;
    };

    struct ImportFunctionInfo
    {
        char ModuleName[MAX_PATH]; // ����ģ������
        DWORD OriginalThunkValue; // OFTsֵ
        DWORD FirstThunkValue;    // FTs (IAT)ֵ
        WORD Hint;            // ������ʾֵ
        BOOL bOrdinal;        // �Ƿ���ŵ���
        WORD Ordinal;         // ���(�������ŵ���)
        char Name[MAX_PATH];  // ��������
    };

    struct ImportTableInfo
    {
        std::vector<ImportModuleInfo> ImportModuleVec;
        std::vector<ImportFunctionInfo> AllImportFunctions;
    };

    struct ExportFuncInfo
    {
        DWORD Ordinal;         // �������
        DWORD FunctionRVA;     // ������ַ
        WORD NameOrdinal;      // ���Ʊ��е����
        DWORD NameRVA;         // ��������RVA
        char Name[MAX_PATH];   // ��������
    };

    struct ExportTableInfo
    {
        IMAGE_EXPORT_DIRECTORY Directory;  
        std::vector<ExportFuncInfo> Functions;
    };

    struct IAT_ENTRY {
        DWORD Address;     // �ڴ��ַ
        char dllName[MAX_PATH];   // ����DLL��
        char funcName[MAX_PATH];   // ������
        WORD Hint;         // ������ʾֵ
        BOOL IsOrdinal;    // �Ƿ���ŵ���
        WORD Ordinal;      // ���ֵ(�������ŵ���)
    };
    std::vector<IAT_ENTRY> m_IatEntries;
    DWORD m_IatVA;  // �û�ָ����IAT��ʼRVA
    DWORD m_IatSize; // �û�ָ����IAT��С

    struct TLSInfo {
        IMAGE_TLS_DIRECTORY Dir;
        std::vector<DWORD> CallbacksVec;
    };

    struct PEStruct
    {
        MY_IMAGE_DOS_HEADER			DOS_HEADER;
        MY_IMAGE_NT_HEADERS			NT_HEADERS;
        MY_IMAGE_SECTION_HEADER* SECTION_HEADERS;

        
        ExportTableInfo ExportInfo;
        ImportTableInfo ImportInfo;
        TLSInfo tlsInfo;
    };


public:
    PEparser(const char* filePath);

    virtual~PEparser();
    void Clean();
    void OpenAgain();
    void OpenFile(const char* filePath);
    
 
    void ParseDos();
    void ParseNTHeaders();
    void ParseSection();
    BOOL ParseImportDir();
    BOOL ParseExportDir();
    BOOL ParseTLSDir();
    void Parse() 
    {
        ParseDos();
        ParseNTHeaders();
        ParseSection();
        ParseImportDir();
        ParseExportDir();
        ParseTLSDir();
    }

    DWORD FOAToRVA(DWORD fileoffset);
    DWORD RVAToFOA(DWORD rva);
    DWORD RVAToVA(DWORD rva);
    DWORD VAToRVA(DWORD va);

    /*
    BOOL MemoryDump(DWORD oepAddress, const char* outputPath);
    BOOL AddSection(const char* secName,DWORD sectionSize);
    BOOL GetIatFormMem(DWORD pid = 0, DWORD startAddress = 0, DWORD size = 0);
    BOOL RebuildIat(const char* filePath);
    BOOL TLSInject(std::vector<BYTE> shellcode);
    */
    //��չ���һ����
    //����½�
    //    ����λ��
    //    �ļ�ĩβ
    //��ڱ� �ռ乻sizeofheaders  �ļ�λ���ж� �ڴ��ж�
    //       �ռ䲻��
    // 

    /*
    
    virtual void SetDataDirectoriesList(CListCtrl& list,  IMAGE_DATA_DIRECTORY dir[], DWORD count) = 0;
   */


    //TODO
    // virtual void SetSingleSectionHeaderList(CListCtrl& list,  IMAGE_SECTION_HEADER& header) = 0;
    //3.PE���ߣ������Զ���ӽ�
    //1.PE�������ӽ��������
     //   2.PE���������Զ�ע��dll���ܡ�


    HANDLE m_hFile;
    HANDLE m_hMapping;
    LPVOID m_pFileBase;
    PEStruct m_PEStruct;
    DWORD m_fileSize;
    CString m_filePath;

    //����
    IMAGE_DOS_HEADER& GetDosHeader();
    IMAGE_NT_HEADERS& GetNtHeader();
    IMAGE_FILE_HEADER& GetFileHeader();
    IMAGE_OPTIONAL_HEADER& GetOptHeader();
    IMAGE_SECTION_HEADER& GetSectionHeader(int index);
    DWORD GetSectionTableOffset();
    DWORD GetDosHeaderOffset();
    DWORD GetNtHeaderOffset();
    DWORD GetFileHeaderOffset();
    DWORD GetOptionalHeaderOffset();
    DWORD GetSectionHeaderOffset(int index);
    //���϶���
    DWORD AlignUp(DWORD value, DWORD alignment);
    DWORD AlignToFile(DWORD value);
    DWORD AlignToSection(DWORD value);

};

