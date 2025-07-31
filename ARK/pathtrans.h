#include "stdafx.h"
#include <windows.h>
#include <iostream>

#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)

#define STATUS_UNSUCCESSFUL              ((NTSTATUS)0xC0000001L)

#define STATUS_SUCCESS                   ((NTSTATUS)0x00000000L)    // ntsubauth

#define STATUS_NAME_TOO_LONG             ((NTSTATUS)0xC0000106L)

//删除指针
#define SafeDeletePoint(pData) { if(pData){delete pData;pData=NULL;} }

//删除数组
#define SafeDeleteArraySize(pData) { if(pData){delete []pData;pData=NULL;} }

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWCH   Buffer;
} UNICODE_STRING;

typedef UNICODE_STRING* PUNICODE_STRING;

typedef struct _RTL_BUFFER {
    PWCHAR    Buffer;
    PWCHAR    StaticBuffer;
    SIZE_T    Size;
    SIZE_T    StaticSize;
    SIZE_T    ReservedForAllocatedSize; // for future doubling
    PVOID     ReservedForIMalloc; // for future pluggable growth
} RTL_BUFFER, * PRTL_BUFFER;

typedef struct _RTL_UNICODE_STRING_BUFFER {
    UNICODE_STRING String;
    RTL_BUFFER     ByteBuffer;
    WCHAR          MinimumStaticBufferForTerminalNul[sizeof(WCHAR)];
} RTL_UNICODE_STRING_BUFFER, * PRTL_UNICODE_STRING_BUFFER;


//DOS路径转换NT路径    C:\\WINDOWS\\system32\\drivers    -- \\??\\C:\\WINDOWS\\system32\\drivers
NTSTATUS DosPathToNtPath(wchar_t* pDosPath, PUNICODE_STRING pNtPath)
{
    //定义变量
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    typedef BOOLEAN(__stdcall* fnRtlDosPathNameToNtPathName_U)(PCWSTR DosFileName, PUNICODE_STRING NtFileName, PWSTR* FilePart, PVOID Reserved);
    static fnRtlDosPathNameToNtPathName_U RtlDosPathNameToNtPathName_U = (fnRtlDosPathNameToNtPathName_U)GetProcAddress(GetModuleHandle(L"ntdll.dll"), "RtlDosPathNameToNtPathName_U");

    //参数效验
    if (IsBadReadPtr(pDosPath, 1) != 0)return NULL;
    if (RtlDosPathNameToNtPathName_U == NULL)return NULL;

    if (RtlDosPathNameToNtPathName_U(pDosPath, pNtPath, NULL, NULL))
    {
        Status = STATUS_SUCCESS;
    }
    return Status;
}

//NT路径转换DOS路径    \\??\\C:\\WINDOWS\\system32\\drivers    -- C:\\WINDOWS\\system32\\drivers
NTSTATUS NtPathToDosPath(PUNICODE_STRING pNtPath, wchar_t* pszDosPath)
{
    //定义变量
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    RTL_UNICODE_STRING_BUFFER DosPath = { 0 };
    wchar_t* ByteDosPathBuffer = NULL;
    wchar_t* ByteNtPathBuffer = NULL;


    typedef NTSTATUS(__stdcall* fnRtlNtPathNameToDosPathName)(ULONG Flags, PRTL_UNICODE_STRING_BUFFER Path, PULONG Disposition, PWSTR* FilePart);
    static fnRtlNtPathNameToDosPathName RtlNtPathNameToDosPathName = (fnRtlNtPathNameToDosPathName)GetProcAddress(GetModuleHandle(L"ntdll.dll"), "RtlNtPathNameToDosPathName");

    //参数效验
    if (IsBadReadPtr(pNtPath, 1) != 0)return Status;
    if (IsBadWritePtr(pszDosPath, 1) != 0)return Status;
    if (RtlNtPathNameToDosPathName == NULL)return Status;

    ByteDosPathBuffer = (wchar_t*)new char[pNtPath->Length + sizeof(wchar_t)];
    ByteNtPathBuffer = (wchar_t*)new char[pNtPath->Length + sizeof(wchar_t)];
    if (ByteDosPathBuffer == NULL || ByteNtPathBuffer == NULL) return Status;

    RtlZeroMemory(ByteDosPathBuffer, pNtPath->Length + sizeof(wchar_t));
    RtlZeroMemory(ByteNtPathBuffer, pNtPath->Length + sizeof(wchar_t));
    RtlCopyMemory(ByteDosPathBuffer, pNtPath->Buffer, pNtPath->Length);
    RtlCopyMemory(ByteNtPathBuffer, pNtPath->Buffer, pNtPath->Length);

    DosPath.ByteBuffer.Buffer = ByteDosPathBuffer;
    DosPath.ByteBuffer.StaticBuffer = ByteNtPathBuffer;
    DosPath.String.Buffer = pNtPath->Buffer;
    DosPath.String.Length = pNtPath->Length;
    DosPath.String.MaximumLength = pNtPath->Length;
    DosPath.ByteBuffer.Size = pNtPath->Length;
    DosPath.ByteBuffer.StaticSize = pNtPath->Length;


    Status = RtlNtPathNameToDosPathName(0, &DosPath, NULL, NULL);
    if (NT_SUCCESS(Status))
    {
        if (_wcsnicmp(pNtPath->Buffer, ByteDosPathBuffer, pNtPath->Length) == 0)
        {
            Status = STATUS_UNSUCCESSFUL;
        }
        else
        {
            RtlCopyMemory(pszDosPath, ByteDosPathBuffer, wcslen(ByteDosPathBuffer) * sizeof(wchar_t));
        }
    }
    else
    {
        //wprintf(L"GetLastError=%i\n", pRtlNtStatusToDosError(Status));
        Status = STATUS_UNSUCCESSFUL;
    }


    SafeDeleteArraySize(ByteDosPathBuffer);
    SafeDeleteArraySize(ByteNtPathBuffer);
    return Status;
}

//\\Device\\HarddiskVolume1\x86.sys    c:\x86.sys    
BOOL DeviceDosPathToNtPath(wchar_t* pszDosPath, wchar_t* pszNtPath)
{
    static TCHAR    szDriveStr[MAX_PATH] = { 0 };
    static TCHAR    szDevName[MAX_PATH] = { 0 };
    TCHAR            szDrive[3];
    INT             cchDevName;
    INT             i;

    //检查参数  
    if (IsBadReadPtr(pszDosPath, 1) != 0)return FALSE;
    if (IsBadWritePtr(pszNtPath, 1) != 0)return FALSE;


    //获取本地磁盘字符串  
    ZeroMemory(szDriveStr, ARRAYSIZE(szDriveStr));
    ZeroMemory(szDevName, ARRAYSIZE(szDevName));
    if (GetLogicalDriveStrings(sizeof(szDriveStr), szDriveStr))
    {
        for (i = 0; szDriveStr[i]; i += 4)
        {
            if (!lstrcmpi(&(szDriveStr[i]), _T("A:\\")) || !lstrcmpi(&(szDriveStr[i]), _T("B:\\")))
                continue;

            szDrive[0] = szDriveStr[i];
            szDrive[1] = szDriveStr[i + 1];
            szDrive[2] = '\0';
            if (!QueryDosDevice(szDrive, szDevName, MAX_PATH))//查询 Dos 设备名  
                return FALSE;

            cchDevName = lstrlen(szDevName);
            if (_tcsnicmp(pszDosPath, szDevName, cchDevName) == 0)//命中  
            {
                lstrcpy(pszNtPath, szDrive);//复制驱动器  
                lstrcat(pszNtPath, pszDosPath + cchDevName);//复制路径  

                return TRUE;
            }
        }
    }

    lstrcpy(pszNtPath, pszDosPath);

    return FALSE;
}

NTSTATUS RtlInitUnicodeString(OUT PUNICODE_STRING DestinationString, IN PCWSTR SourceString)
{

#define ARGUMENT_PRESENT(ArgumentPointer)    (\
    (CHAR *)((ULONG_PTR)(ArgumentPointer)) != (CHAR *)(NULL) )

    SIZE_T Length;
    DestinationString->Length = 0;
    DestinationString->MaximumLength = 0;
    DestinationString->Buffer = (PWSTR)SourceString;
    if (ARGUMENT_PRESENT(SourceString))
    {
        Length = wcslen(SourceString);

        // We are actually limited to 32765 characters since we want to store a meaningful
        // MaximumLength also.
        if (Length > (UNICODE_STRING_MAX_CHARS - 1)) {
            return STATUS_NAME_TOO_LONG;
        }

        Length *= sizeof(WCHAR);

        DestinationString->Length = (USHORT)Length;
        DestinationString->MaximumLength = (USHORT)(Length + sizeof(WCHAR));
    }

    return STATUS_SUCCESS;
}
