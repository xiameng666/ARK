#pragma once
#define MAX_PATH 256

#define DEVICE_NAME L"\\Device\\ADriver1"
#define SYMBOL_NAME L"\\DosDevices\\ADriver1"
#define DOS_NAME     L"\\\\\\.\\ADriver1"

typedef struct _KERNEL_SYMBOLS {
    // 地址类（VA）
    ULONG_PTR ProcessListHead;      // PsActiveProcessHead
    ULONG_PTR SystemProcess;        // PsInitialSystemProcess 
    ULONG_PTR SSDTBase;             // KeServiceDescriptorTable

    // 偏移类（OFFSET）
    ULONG ProcessLinks;            // ActiveProcessLinks偏移
    ULONG ProcessId;               // UniqueProcessId偏移
    ULONG ProcessName;             // ImageFileName偏移
    ULONG ParentId;                // InheritedFromUniqueProcessId偏移
} KERNEL_SYMBOLS;

#define GET_PROCESS_LIST_HEAD()     ((PLIST_ENTRY)g_KernelSymbols.ProcessListHead)
#define GET_PROCESS_LINKS_OFFSET()  (g_KernelSymbols.ProcessLinks)
#define GET_PROCESS_ID_OFFSET()     (g_KernelSymbols.ProcessId)

typedef enum : ULONG {
    HOOK_CREATE_PROCESS = 48,     // NtCreateProcessEx
    HOOK_OPEN_PROCESS = 122,      // NtOpenProcess
    HOOK_DEBUG_PROCESS = 57,      // NtDebugActiveProcess
    HOOK_SUSPEND_PROCESS = 253,   // NtSuspendProcess
    HOOK_RESUME_PROCESS = 205,    // NtResumeProcess 
    HOOK_TERMINATE_PROCESS = 257  // NtTerminateProcess
} HOOK_SSDT_Index, * PHOOK_SSDT_Index;

typedef struct PROCESS_EVENT {
    CHAR SourceProcessName[16];        // 调用者进程名
    ULONG SourceProcessId;             // 调用者进程ID
    ULONG Action;                      // 动作 (HOOK_SSDT_Index)
    ULONG TargetProcessId;             // targetPID
    LONG Result;                       // 操作结果
} PROCESS_EVENT, * PPROCESS_EVENT;

// 缓冲区结构
typedef struct _LOG_BUFFER {
    volatile ULONG WriteIndex;     // 写入索引（R0更新）
    volatile ULONG ReadIndex;      // 读取索引（R3更新）
    volatile ULONG LogCount;       // 当前日志数量
    PROCESS_EVENT Logs[1000];      // 日志数组
} LOG_BUFFER, *PLOG_BUFFER;


enum WindowsVersion
{
    WinXP,
    Win7,
    Other
};

#define MY_CTL_CODE(code)           CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800 + code, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define CTL_READ_MEM                MY_CTL_CODE(0)
#define CTL_WRITE_MEM               MY_CTL_CODE(1)


#define CTL_ATTACH_MEM_READ         MY_CTL_CODE(2)      //附加进程读
#define CTL_ATTACH_MEM_WRITE        MY_CTL_CODE(3)      //写


#define CTL_GET_GDT_DATA            MY_CTL_CODE(10)     //获取GDT表数据

#define CTL_ENUM_PROCESS_COUNT      MY_CTL_CODE(20)     // 枚举进程 返回数量
#define CTL_ENUM_PROCESS            MY_CTL_CODE(21)     // 返回数据
#define CTL_KILL_PROCESS            MY_CTL_CODE(22)     // 终止进程

#define CTL_ENUM_MODULE_COUNT       MY_CTL_CODE(30)     // 枚举模块 返回数量
#define CTL_ENUM_MODULE             MY_CTL_CODE(31)     // 枚举模块 返回数据

#define CTL_ENUM_PROCESS_MODULE_COUNT MY_CTL_CODE(32)   // 枚举进程模块 返回数量
#define CTL_ENUM_PROCESS_MODULE       MY_CTL_CODE(33)   // 枚举进程模块 返回数据

#define CTL_ENUM_SSDT               MY_CTL_CODE(35)     // 枚举SSDT 返回数据  假定max 500条记录 不要返回数量了

#define CTL_START_SSDTHOOK          MY_CTL_CODE(40)     //开始监控 R3发ssdt的index  R0返回映射到R3的内存地址
#define CTL_END_SSDTHOOK            MY_CTL_CODE(41)     //结束监控 R3发ssdt的index  

#define CTL_SET_PDB_PATH            MY_CTL_CODE(50)     // 设置PDB下载路径

// PDB路径设置结构体
typedef struct _PDB_PATH_REQUEST {
    wchar_t DownloadPath[MAX_PATH];    // PDB下载路径
} PDB_PATH_REQUEST, *PPDB_PATH_REQUEST;

//#define CTL_ENUM_DRIVER_COUNT       MY_CTL_CODE(40)
//#define CTL_ENUM_DRIVER             MY_CTL_CODE(41)   // 枚举驱动
//
//#define CTL_ENUM_THREAD_COUNT       MY_CTL_CODE(50)
//#define CTL_ENUM_THREAD             MY_CTL_CODE(51)   // 枚举线程
//
//#define CTL_ENUM_HANDLE_COUNT       MY_CTL_CODE(60)
//#define CTL_ENUM_HANDLE             MY_CTL_CODE(61)   // 枚举句柄
//#define CTL_QUERY_SYSINFO           MY_CTL_CODE(62)   // 查询系统信息
//
//#define CTL_ENUM_REGISTRY_COUNT     MY_CTL_CODE(70)
//#define CTL_ENUM_REGISTRY           MY_CTL_CODE(71)   // 枚举注册表项
//#define CTL_READ_REGISTRY           MY_CTL_CODE(72)  // 读取注册表
//#define CTL_WRITE_REGISTRY          MY_CTL_CODE(73)  // 写注册表
//
//#define CTL_ENUM_CALLBACK_COUNT     MY_CTL_CODE(80)
//#define CTL_ENUM_CALLBACK           MY_CTL_CODE(81)  // 枚举内核回调
//
//#define CTL_ENUM_HOOK_COUNT         MY_CTL_CODE(90)
//#define CTL_ENUM_HOOK               MY_CTL_CODE(91)  // 



typedef struct KERNEL_RW_REQ {
    unsigned Address;
    unsigned Size;
    unsigned char Buffer[256];
}*PKERNEL_RW_REQ;

//读写目标进程内存
typedef struct PROCESS_MEM_REQ {
    HANDLE ProcessId;        // 目标进程ID
    PVOID VirtualAddress;    // 虚拟地址
    unsigned Size;           // 数据大小  
                             //写到systembuffer 不需要在结构体定义缓冲区
}*PPROCESS_MEM_REQ;

#pragma pack(push, 1)
typedef struct GDTR {
  unsigned short Limit;
  ULONG_PTR Base;
}*PGDTR;
#pragma pack(pop)

typedef struct GDT_DATA_REQ {
    unsigned CpuIndex;
    GDTR Gdtr;
}*PGDT_DATA_REQ;

typedef struct PROCESS_INFO {
    ULONG ProcessId;                    // 进程ID
    ULONG ParentProcessId;              // 父进程ID
    CHAR ImageFileName[16];             // 进程名称（短名）
    PVOID EprocessAddr;                 // EPROCESS地址
    ULONG DirectoryTableBase;           // CR3页目录基地址  

}*PPROCESS_INFO;

typedef struct MODULE_INFO {
    CHAR Name[64];                      // 模块名称（短名）
    CHAR FullPath[256];                 // 模块完整路径
    PVOID ImageBase;                    // 模块基地址
    ULONG ImageSize;                    // 模块大小
    USHORT LoadOrderIndex;              // 加载顺序索引
    USHORT LoadCount;                   // 加载计数

    //CHAR Company[128];                // 厂商信息（暂时为空，需要解析PE）
}*PMODULE_INFO;

typedef struct PROCESS_MODULE_REQ {
    HANDLE ProcessId;                   // 目标进程ID
    ULONG ModuleCount;                  // 模块数量（输出参数）
}*PPROCESS_MODULE_REQ;

typedef struct SegmentDescriptor {
    unsigned Limit1 : 16;    // 界限低16位
    unsigned Base1 : 16;     // 基址低16位  
    unsigned Base2 : 8;      // 基址中8位
    unsigned type : 4;       // 段类型
    unsigned s : 1;          // 系统段标志
    unsigned dpl : 2;        // 特权级
    unsigned p : 1;          // 存在位
    unsigned Limit2 : 4;     // 界限高4位
    unsigned avl : 1;        // 软件可用位
    unsigned l : 1;          // 64位代码段标志
    unsigned db : 1;         // 操作数大小
    unsigned g : 1;          // 粒度位
    unsigned Base3 : 8;      // 基址高8位
} SegmentDescriptor, *PSEGDESC;  // 确保是8字节

// 64位系统段描述符（16字节）
typedef struct SystemDescriptor64 {
    SegmentDescriptor low;   // 低8字节
    unsigned Base4 : 32;     // 基址最高32位
    unsigned reserved : 32;  // 保留字段
} SystemDescriptor64;


typedef struct SSDT_INFO {
    ULONG Index;
    PVOID FunctionAddress;
    CHAR FunctionName[64];
}*PSSDT_INFO;

typedef struct _SYSTEM_SERVICE_DESCRIPTOR_TABLE {
    PULONG Base;      // 系统服务函数指针数组
    PULONG ServiceCounterTable;   // 服务调用计数表
    ULONG NumberOfServices;       // 服务数量
    PUCHAR ParamTableBase;        // 参数表
} SYSTEM_SERVICE_DESCRIPTOR_TABLE, * PSYSTEM_SERVICE_DESCRIPTOR_TABLE;


