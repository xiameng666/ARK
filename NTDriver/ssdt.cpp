#include "ssdt.h"

ULONG_PTR SSDT_GetPfnAddr(ULONG dwIndex, PULONG lpBase)//https://bbs.kanxue.com/thread-248117.htm
{
    ULONG_PTR lpAddr = NULL;

    ULONG dwOffset = lpBase[dwIndex];

    //按16位对齐省空间，所以>>4;负偏移有+-问题，所以|0xF00..
    if (dwOffset & 0x80000000)
        dwOffset = (dwOffset >> 4) | 0xF0000000;
    else
        dwOffset >>= 4;

    lpAddr = (ULONG_PTR)((PUCHAR)lpBase + (LONG)dwOffset);

    return lpAddr;
}

NTSTATUS EnumSSDT(PSSDT_INFO SsdtBuffer, PULONG SsdtCount)//X64的SSDT是rva
{
    INIT_PDB;
    PSYSTEM_SERVICE_DESCRIPTOR_TABLE KeServiceDescriptorTable = (PSYSTEM_SERVICE_DESCRIPTOR_TABLE)ntos.GetPointer("KeServiceDescriptorTable");
    
    if (!KeServiceDescriptorTable) {
        Log("[XM] KeServiceDescriptorTable == null");
        return STATUS_UNSUCCESSFUL;
    }

    ULONG nums = KeServiceDescriptorTable->NumberOfServices;
    PULONG ssdt = KeServiceDescriptorTable->Base;
    *SsdtCount = nums;

    for (ULONG i = 0; i < nums; i++) {
        SsdtBuffer[i].Index = i;

        ULONG_PTR pfnAddr = SSDT_GetPfnAddr(i, ssdt);
        SsdtBuffer[i].FunctionAddress = (PVOID)pfnAddr;

        const char* functionName = ntos.GetNameByVA(pfnAddr);
        Log("[XM] GetName result: %s", functionName ? functionName : "(null)");

        if (functionName && strlen(functionName) > 0) {
            strcpy_s(SsdtBuffer[i].FunctionName, sizeof(SsdtBuffer[i].FunctionName), functionName);
        }
        else {
            // 如果没找到，使用索引号
            sprintf_s(SsdtBuffer[i].FunctionName, sizeof(SsdtBuffer[i].FunctionName), "Nt#%d", i);
        }
    }

    return STATUS_SUCCESS;
}

NTSTATUS EnumShadowSSDT(PSSDT_INFO SsdtBuffer, PULONG SsdtCount)
{
    INIT_PDB;
    PSYSTEM_SERVICE_DESCRIPTOR_TABLE ShadowTable =
        (PSYSTEM_SERVICE_DESCRIPTOR_TABLE)ntos.GetPointer("KeServiceDescriptorTableShadow");

    Log("[XM] KeServiceDescriptorTableShadow ： %p", ShadowTable);

    if (!ShadowTable) {
        Log("[XM] KeServiceDescriptorTableShadow == null");
        return STATUS_UNSUCCESSFUL;
    }

    if (!ShadowTable->Base || ShadowTable->NumberOfServices == 0) {
        Log("[XM] ShadowSSDT not available");
        *SsdtCount = 0;
        return STATUS_SUCCESS; 
    }

    ULONG nums = ShadowTable->NumberOfServices;
    PULONG shadowSsdt = ShadowTable->Base;
    *SsdtCount = nums;

    Log("[XM] ShadowSSDT found: %d services", nums);

    for (ULONG i = 0; i < nums; i++) {
        SsdtBuffer[i].Index = i + 0x1000;  // ShadowSSDT的调用号从0x1000开始

        // ShadowSSDT存储的是相对于win32k.sys的RVA，不需要解码
        ULONG rva = shadowSsdt[i];
        SsdtBuffer[i].FunctionAddress = (PVOID)(ULONG_PTR)rva;  // 传递RVA到R3

        //函数名在R3解析（R3会用win32k_base + RVA计算真实地址）
        sprintf_s(SsdtBuffer[i].FunctionName, sizeof(SsdtBuffer[i].FunctionName), "Win32k#%d", i);

        Log("[XM] ShadowSSDT[%d]: RVA=0x%X", i, rva);
    }

    return STATUS_SUCCESS;
}
