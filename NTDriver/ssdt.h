#pragma once
#include "mydef.h"

extern "C" {

    NTSTATUS EnumSSDTFromMem(PSSDT_INFO SsdtBuffer, PULONG SsdtCount);

    NTSTATUS RecoverSSDT();

    NTSTATUS RecoverShadowSSDT();
    NTSTATUS RecoverShadowSSDT_Win7();
    NTSTATUS RecoverShadowSSDT_win32u();

    NTSTATUS EnumShadowSSDT(PSSDT_INFO SsdtBuffer, PULONG SsdtCount);

    ULONG_PTR SSDT_GetPfnAddr(ULONG dwIndex, PULONG lpBase);

}
