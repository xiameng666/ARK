
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
        char ModuleName[MAX_PATH]; // 所属模块名称
        DWORD OriginalThunkValue; // OFTs值
        DWORD FirstThunkValue;    // FTs (IAT)值
        WORD Hint;            // 函数提示值
        BOOL bOrdinal;        // 是否按序号导入
        WORD Ordinal;         // 序号(如果按序号导入)
        char Name[MAX_PATH];  // 函数名称
    };

    struct ImportTableInfo
    {
        std::vector<ImportModuleInfo> ImportModuleVec;
        std::vector<ImportFunctionInfo> AllImportFunctions;
    };

    struct ExportFuncInfo
    {
        DWORD Ordinal;         // 函数序号
        DWORD FunctionRVA;     // 函数地址
        WORD NameOrdinal;      // 名称表中的序号
        DWORD NameRVA;         // 函数名称RVA
        char Name[MAX_PATH];   // 函数名称
    };

    struct ExportTableInfo
    {
        IMAGE_EXPORT_DIRECTORY Directory;  
        std::vector<ExportFuncInfo> Functions;
    };

    struct IAT_ENTRY {
        DWORD Address;     // 内存地址
        char dllName[MAX_PATH];   // 所属DLL名
        char funcName[MAX_PATH];   // 函数名
        WORD Hint;         // 函数提示值
        BOOL IsOrdinal;    // 是否按序号导入
        WORD Ordinal;      // 序号值(如果按序号导入)
    };
    std::vector<IAT_ENTRY> m_IatEntries;
    DWORD m_IatVA;  // 用户指定的IAT起始RVA
    DWORD m_IatSize; // 用户指定的IAT大小

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
    //扩展最后一个节
    //添加新节
    //    空闲位置
    //    文件末尾
    //插节表 空间够sizeofheaders  文件位置判断 内存判断
    //       空间不够
    // 

    /*
    
    virtual void SetDataDirectoriesList(CListCtrl& list,  IMAGE_DATA_DIRECTORY dir[], DWORD count) = 0;
   */


    //TODO
    // virtual void SetSingleSectionHeaderList(CListCtrl& list,  IMAGE_SECTION_HEADER& header) = 0;
    //3.PE工具，增加自动添加节
    //1.PE工具增加解析导入表。
     //   2.PE工具增加自动注入dll功能。


    HANDLE m_hFile;
    HANDLE m_hMapping;
    LPVOID m_pFileBase;
    PEStruct m_PEStruct;
    DWORD m_fileSize;
    CString m_filePath;

    //辅助
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
    //向上对齐
    DWORD AlignUp(DWORD value, DWORD alignment);
    DWORD AlignToFile(DWORD value);
    DWORD AlignToSection(DWORD value);

};

