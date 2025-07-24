#pragma once
#include "mydef.h"

extern "C" {

    NTSTATUS EnumSSDT(PSSDT_INFO SsdtBuffer, PULONG SsdtCount);

    ULONG_PTR SSDT_GetPfnAddr(ULONG dwIndex, PULONG lpBase);
}
