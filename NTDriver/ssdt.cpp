#include "ssdt.h"

ULONG_PTR SSDT_GetPfnAddr(ULONG dwIndex, PULONG lpBase)//https://bbs.kanxue.com/thread-248117.htm
{
    ULONG_PTR lpAddr = NULL;

    ULONG dwOffset = lpBase[dwIndex];

    // SAR这个指令, 以及右移4位, 决定了0xF0000000这个值。
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
    Log("[XM] KeServiceDescriptorTable:%p", KeServiceDescriptorTable);
    if (!KeServiceDescriptorTable)
        return STATUS_UNSUCCESSFUL;

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
