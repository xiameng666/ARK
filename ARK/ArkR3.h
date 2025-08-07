#pragma once

#include "driverloader.h"
#include"../include/proto.h"
#include "ezpdb.hpp"

#include <stdio.h>
#include <vector>
#include <memory>
#include <tlhelp32.h>
#include <string>
#include <immintrin.h>
#include <winternl.h>
#include <Shlobj.h>
#include <PathCch.h>
#include <set>
#include"pestruct.h"

#ifdef _WIN64
extern "C" void _sgdt(void*);
#endif

typedef struct GDT_INFO {
    UINT    cpuIndex;       // CPU序号
    USHORT  selector;       // 段选择子
    ULONG64 base;           // 段基址（64位，Base1+Base2+Base3拼接，系统段需合并高32位）
    ULONG   limit;          // 段界限（Limit1+Limit2拼接，g=1时需左移12位并或0xFFF）
    UCHAR   g;              // 段粒度（0=字节粒度，1=4KB页粒度）
    UCHAR   dpl;            // 段特权级（0=Ring0内核，3=Ring3用户）
    UCHAR   type;           // 段类型
    UCHAR   system;         // 系统段标志（0=系统段，1=应用段）
    BOOL    p;              // 段存在位（Present，1=有效，0=无效）
    UCHAR   l;              // 64位代码段标志（L位，1=64位代码段，仅代码段有效，x64特有）
    BOOL    is_system_64;   // 是否为64位系统段（如x64下TSS/LDT扩展段，需合并16字节）
    char    typeDesc[32];   // 段类型描述字符串（如"Code (R E)", "32-bit TSS"等，便于UI显示）
} *PGDT_INFO;

class ArkR3 :public DriverLoader
{
private:

    //附加进程读写相关
    PVOID memBuffer_;
    DWORD memBufferSize_;
    DWORD memDataSize_;

    //符号信息相关
    ULONG_PTR ntbase_ = 0;              // ntoskrnl.exe基址
    std::string ntos_path_;             // ntoskrnl.exe路径
    std::unique_ptr<ez::pdb> ntos_pdb_; // PDB对象

    //win32k符号
    ULONG_PTR win32k_base_= 0;
    std::string win32k_path_;
    std::unique_ptr<ez::pdb> win32k_pdb_;

public:
    ArkR3();
    ~ArkR3();

    void SendPdbInfo();

    // 从ezpdb获取PDB路径并设置给驱动
    bool SetPdbPathFromEzpdb();
    
    bool InitSymbolState();                             //初始化符号信息到成员变量
    ULONG_PTR GetSSDTBaseRVA();
    void GetFileSSDT();
    bool RestoreSSdt();
    ULONG_PTR GetModuleBase(const char* moduleName);    //NtApi获取模块基址
    ULONG_PTR GetKernelSymbolVA(const char* symbolName);
    ULONG GetKernelSymbolOffset(const char* structName, const wchar_t* fieldName);
    //ntbase_+rva
    BOOL SendVA(ULONG_PTR va);                          //WriteFile发送VA到R0的g_VA

    //修复模块中名称中systemroot等信息
    std::wstring FixModulePath(const std::wstring& path);
    std::string FixModulePath(const std::string& path);

    PSEGDESC GDTGetSingle(UINT cpuIndex, PGDTR pGdtr, DWORD* pRetBytes); //获得单核GDT表数据指针
    std::vector<GDT_INFO> GDTGetVec();                                   //返回所有核心GDT数组_gdtVec
    std::vector<GDT_INFO> GDTVec_;

    DWORD IdtGetCount();
    std::vector<IDT_INFO> IdtGetVec();
    std::vector<IDT_INFO> IDTVec_;

    BOOL MemAttachRead(DWORD ProcessId, ULONG_PTR VirtualAddress, DWORD Size); //附加读
    BOOL MemAttachWrite(DWORD ProcessId, ULONG_PTR VirtualAddress, DWORD Size);

    //附加写
    BOOL MemEnsureBufferSize(DWORD requiredSize);                          //确保缓冲区大小
    void MemClearBuffer();                                                 //清空内存读写的缓冲区
    PVOID GetBufferData() const { return memBuffer_; }
    DWORD GetDataSize() const { return memDataSize_; }
    DWORD GetBufferSize() const { return memBufferSize_; }

    //进程
    BOOL ProcessForceKill(ULONG ProcessId);
    DWORD ProcessGetCount();                                                //获取进程数量
    std::vector<PROCESS_INFO> ProcessGetVec(DWORD processCount = 0);        //返回所有进程数据的数组ProcVec_
    std::vector<PROCESS_INFO> ProcessSearchGetVec();
    std::vector<PROCESS_INFO> ProcSearchVec_;
    std::vector<PROCESS_INFO> ProcVec_;
    //std::vector<PROCESSENTRY32> EnumProcesses32();                        //R3的枚举进程

    DWORD ModuleGetCount();
    std::vector<MODULE_INFO> ModuleGetVec(DWORD moduleCount = 0);
    std::vector<MODULE_INFO> MoudleVec_;
    
    // 进程模块枚举
    DWORD ProcessModuleGetCount(DWORD processId);
    std::vector<MODULE_INFO> ProcessModuleGetVec(DWORD processId, DWORD moduleCount = 0);
    std::vector<MODULE_INFO> ProcessModuleVec_;

    //SSDT
    std::vector<SSDT_INFO> SSDTGetVec();
    std::string GetWin32kFunctionName(ULONG_PTR address);
    std::vector<SSDT_INFO> SSDTVec_;

    //ShadowSSDT
    std::vector<ShadowSSDT_INFO> ShadowSSDTGetVec();
    std::vector<ShadowSSDT_INFO> ShadowSSDTVec_;

    // 回调相关
    std::vector<CALLBACK_INFO> CallbackGetVec(CALLBACK_TYPE type);      // 获取指定类型的回调列表
    BOOL CallbackDelete(CALLBACK_TYPE type, ULONG index, PVOID CallbackFuncAddr);
    std::vector<CALLBACK_INFO> CallbackVec_;                            // 回调数据缓存

    // 派遣函数Hook检测
    std::vector<DISPATCH_HOOK_INFO> DispatchHookGetVec();               // 获取派遣函数Hook信息
    std::vector<DISPATCH_HOOK_INFO> DispatchHookVec_;                   // Hook检测结果缓存
    
    // 设备栈分析
    std::vector<DEVICE_STACK_INFO> DeviceStackGetVec();                 // 获取设备栈分析信息
    std::vector<DEVICE_STACK_INFO> DeviceStackVec_;                     // 设备栈分析结果缓存

    //网络
    void GetTcpStateString(DWORD dwState, char* stateStr, size_t stateSize);
    std::vector<NETWORK_PORT_INFO> NetworkPortGetVec();                 // 获取网络端口相关信息
    std::vector<NETWORK_PORT_INFO> NetworkPortVec_;                     // 网络端口信息缓存

    //文件
    BOOL ForceDeleteFile(const std::string& filePath);
    BOOL UnlockFile(const std::string& filePath);
    
    // SSDTHOOK 
    BOOL StartSSDTHook(HOOK_SSDT_Index flag);
    BOOL EndSSDTHook(HOOK_SSDT_Index flag);

    std::vector<PROCESS_EVENT> ProcessEventsVec_;
    BOOL isHookThreadRunning_ = FALSE;
    PLOG_BUFFER pSharedLogBuffer_ = NULL;       //映射到的R3内存地址
    void ArkR3::ReadProcessEvents();

    static DWORD WINAPI LogThreadProc(LPVOID lpParam);
    HANDLE hLogThread_ = NULL;
};

typedef struct _RTL_PROCESS_MODULE_INFORMATION {
    HANDLE Section;
    PVOID  MappedBase;
    PVOID  ImageBase;
    ULONG  ImageSize;
    ULONG  Flags;
    USHORT LoadOrderIndex;
    USHORT InitOrderIndex;
    USHORT LoadCount;
    USHORT OffsetToFileName;
    UCHAR  FullPathName[256];
} RTL_PROCESS_MODULE_INFORMATION, * PRTL_PROCESS_MODULE_INFORMATION;

typedef struct _RTL_PROCESS_MODULES {
    ULONG NumberOfModules;
    RTL_PROCESS_MODULE_INFORMATION Modules[1];
} RTL_PROCESS_MODULES, * PRTL_PROCESS_MODULES;

