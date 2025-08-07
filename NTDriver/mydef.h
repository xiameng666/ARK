#pragma once

#ifndef MAX_PATH
#define MAX_PATH 260
#endif // !MAX_PATH

#include <ntifs.h>
#include <ntddk.h>
#include "../include/proto.h"
#include <ntstrsafe.h>  
#include <intrin.h>
#include "DriverBase.h"
#include "oxygenPdb.h"
#include "process.h"
#include "module.h"

extern wchar_t g_ntosPdbPath[512];
extern wchar_t g_win32kPdbPath[512];

/*
    在任何函数内获取VA
    INIT_NTOS;
    OFFSET(ULONG类型的对象 "_ETHREAD", "ThreadsProcess");
*/
#define INIT_NTOS \
    oxygenPdb::Pdber ntos(L"ntoskrnl.exe"); \
     ntos.setPdbDownloadPath(g_ntosPdbPath); \
    ntos.init();

#define INIT_WIN32K \
    oxygenPdb::Pdber win32k(L"win32k.sys"); \
     win32k.setPdbDownloadPath(g_win32kPdbPath); \
    win32k.init();

#define OFFSET(value, structName, memberName) value = ((ULONG)ntos.GetOffset((structName), (memberName)))


