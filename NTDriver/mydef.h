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

extern wchar_t g_PdbDownloadPath[512];

/*
    ���κκ����ڻ�ȡVA
    INIT_PDB;
    OFFSET(ULONG���͵Ķ��� "_ETHREAD", "ThreadsProcess");
*/
#define INIT_PDB \
    oxygenPdb::Pdber ntos(L"ntoskrnl.exe"); \
     ntos.setPdbDownloadPath(g_PdbDownloadPath); \
    ntos.init();

#define OFFSET(value, structName, memberName) value = ((ULONG)ntos.GetOffset((structName), (memberName)))


