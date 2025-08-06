#pragma once
#include "mydef.h"

extern "C" {

    NTSTATUS EnumSSDTFormMem(PSSDT_INFO SsdtBuffer, PULONG SsdtCount);

    NTSTATUS EnumSSDTFromFile(PSSDT_INFO SsdtBuffer, PULONG SsdtCount);

    NTSTATUS EnumShadowSSDT(PSSDT_INFO SsdtBuffer, PULONG SsdtCount);

    ULONG_PTR SSDT_GetPfnAddr(ULONG dwIndex, PULONG lpBase);
}
