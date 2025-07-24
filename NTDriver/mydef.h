#pragma once
#define MAX_PATH 256

#include <ntifs.h>
#include <ntddk.h>
#include "../include/proto.h"
#include <ntstrsafe.h>  
#include <intrin.h>
#include "driver.h"
#include "oxygenPdb.h"
#include "process.h"
#include "module.h"

extern wchar_t g_PdbDownloadPath[256];
extern BOOLEAN g_HasCustomPdbPath;

#define INIT_PDB \
    oxygenPdb::Pdber ntos(L"ntoskrnl.exe"); \
    if (g_HasCustomPdbPath) { \
        ntos.setPdbDownloadPath(g_PdbDownloadPath); \
    } \
    ntos.init();

#define OFFSET(value, structName, memberName) value = ((ULONG)ntos.GetOffset((structName), (memberName)))

