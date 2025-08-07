#include "mydef.h"
#include <ntimage.h>

NTSTATUS KernelMapFile(UNICODE_STRING FileName, HANDLE* phFile, HANDLE* phSection, PVOID* ppBaseAddress)
{
    NTSTATUS status = STATUS_SUCCESS;
    HANDLE hFile = NULL;
    HANDLE hSection = NULL;
    OBJECT_ATTRIBUTES objectAttr = { 0 };
    IO_STATUS_BLOCK iosb = { 0 };
    PVOID pBaseAddress = NULL;
    SIZE_T viewSize = 0;

    // 设置文件权限
    InitializeObjectAttributes(&objectAttr, &FileName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

    // 打开文件
    status = ZwOpenFile(&hFile, GENERIC_READ, &objectAttr, &iosb, FILE_SHARE_READ, FILE_SYNCHRONOUS_IO_NONALERT);
    if (!NT_SUCCESS(status))
    {
        return status;
    }

    // 创建节对象
    status = ZwCreateSection(&hSection, SECTION_MAP_READ | SECTION_MAP_WRITE, NULL, 0, PAGE_READWRITE, 0x1000000, hFile);
    if (!NT_SUCCESS(status))
    {
        ZwClose(hFile);
        return status;
    }
    // 映射到内存
    status = ZwMapViewOfSection(hSection, NtCurrentProcess(), &pBaseAddress, 0, 1024, 0, &viewSize, ViewShare, MEM_TOP_DOWN, PAGE_READWRITE);
    if (!NT_SUCCESS(status))
    {
        ZwClose(hSection);
        ZwClose(hFile);
        return status;
    }

    // 返回数据
    *phFile = hFile;
    *phSection = hSection;
    *ppBaseAddress = pBaseAddress;

    return status;
}

//重定位RVA

//RVA与FOA转换

// 寻找指定函数得到内存地址  这个函数得到的是什么地址
ULONG64 GetAddressFromFunction(UNICODE_STRING DllFileName, PCHAR pszFunctionName)
{
    NTSTATUS status = STATUS_SUCCESS;
    HANDLE hFile = NULL;
    HANDLE hSection = NULL;
    PVOID pBaseAddress = NULL;

    // 内存映射文件
    status = KernelMapFile(DllFileName, &hFile, &hSection, &pBaseAddress);
    if (!NT_SUCCESS(status))
    {
        return 0;
    }
    PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pBaseAddress;
    PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)((PUCHAR)pDosHeader + pDosHeader->e_lfanew);
    PIMAGE_EXPORT_DIRECTORY pExportTable = (PIMAGE_EXPORT_DIRECTORY)((PUCHAR)pDosHeader + pNtHeaders->OptionalHeader.DataDirectory[0].VirtualAddress);
    ULONG ulNumberOfNames = pExportTable->NumberOfNames;
    PULONG lpNameArray = (PULONG)((PUCHAR)pDosHeader + pExportTable->AddressOfNames);
    PCHAR lpName = NULL;

    for (ULONG i = 0; i < ulNumberOfNames; i++)
    {
        lpName = (PCHAR)((PUCHAR)pDosHeader + lpNameArray[i]);
        USHORT uHint = *(USHORT*)((PUCHAR)pDosHeader + pExportTable->AddressOfNameOrdinals + 2 * i);
        ULONG ulFuncAddr = *(PULONG)((PUCHAR)pDosHeader + pExportTable->AddressOfFunctions + 4 * uHint);
        PVOID lpFuncAddr = (PVOID)((PUCHAR)pDosHeader + ulFuncAddr);

        if (_strnicmp(pszFunctionName, lpName, strlen(pszFunctionName)) == 0)
        {
            ZwUnmapViewOfSection(NtCurrentProcess(), pBaseAddress);
            ZwClose(hSection);
            ZwClose(hFile);

            return (ULONG64)lpFuncAddr;
        }
    }
    ZwUnmapViewOfSection(NtCurrentProcess(), pBaseAddress);
    ZwClose(hSection);
    ZwClose(hFile);
    return 0;
}

ULONG_PTR PeReverseRelocateValue(UNICODE_STRING* FileName, ULONG_PTR RelocatedValue, ULONG_PTR RuntimeBase) {
    NTSTATUS status;
    HANDLE hFile = NULL;
    HANDLE hSection = NULL;
    PVOID pBaseAddress = NULL;
    ULONG_PTR result = RelocatedValue;

    // 映射文件
    status = KernelMapFile(*FileName, &hFile, &hSection, &pBaseAddress);
    if (!NT_SUCCESS(status)) {
        return RelocatedValue;
    }

    PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pBaseAddress;
    PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)((PUCHAR)pDosHeader + pDosHeader->e_lfanew);

    // 获取文件ImageBase和重定位表
    ULONG_PTR fileImageBase = pNtHeaders->OptionalHeader.ImageBase;
    PIMAGE_DATA_DIRECTORY pRelocDir = &pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];

    if (pRelocDir->VirtualAddress == 0) {
        // 无重定位表，简单计算
        result = RelocatedValue - (RuntimeBase - fileImageBase);
        goto cleanup;
    }

    PIMAGE_BASE_RELOCATION pReloc = (PIMAGE_BASE_RELOCATION)((PUCHAR)pDosHeader + pRelocDir->VirtualAddress);
    ULONG relocSize = pRelocDir->Size;
    ULONG_PTR runtimeRVA = RelocatedValue - RuntimeBase;

    // 遍历重定位表
    while ((PUCHAR)pReloc - (PUCHAR)pDosHeader - pRelocDir->VirtualAddress < relocSize && pReloc->SizeOfBlock > 0) {

        if (runtimeRVA >= pReloc->VirtualAddress && runtimeRVA < pReloc->VirtualAddress + 0x1000) {
            size_t relocCount = (pReloc->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(USHORT);
            PUSHORT pRelocItems = (PUSHORT)((PUCHAR)pReloc + sizeof(IMAGE_BASE_RELOCATION));

            for (size_t i = 0; i < relocCount; i++) {
                USHORT item = pRelocItems[i];
                USHORT offset = item & 0xFFF;
                ULONG_PTR relocRVA = pReloc->VirtualAddress + offset;

                if (runtimeRVA == relocRVA) {
                    // 找到重定位项，读取文件中的原始值
                    ULONG_PTR originalValue = *(PULONG_PTR)((PUCHAR)pDosHeader + relocRVA);
                    result = originalValue;
                    goto cleanup;
                }
            }
        }

        pReloc = (PIMAGE_BASE_RELOCATION)((PUCHAR)pReloc + pReloc->SizeOfBlock);
    }

    // 未找到重定位项，简单计算
    result = RelocatedValue - (RuntimeBase - fileImageBase);

cleanup:
    if (pBaseAddress) ZwUnmapViewOfSection(NtCurrentProcess(), pBaseAddress);
    if (hSection) ZwClose(hSection);
    if (hFile) ZwClose(hFile);

    return result;
}

ULONG_PTR RVA2FOA(PVOID pFileBase, ULONG_PTR Rva) {
    if (!pFileBase || Rva == 0) return 0;

    PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pFileBase;
    PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)((PUCHAR)pDosHeader + pDosHeader->e_lfanew);
    PIMAGE_SECTION_HEADER pSectionHeader = IMAGE_FIRST_SECTION(pNtHeaders);
    USHORT sectionCount = pNtHeaders->FileHeader.NumberOfSections;

    Log("[PE] RVA2FOA: 查找RVA=0x%x，总节数=%d", Rva, sectionCount);

    // 遍历所有节
    for (USHORT i = 0; i < sectionCount; i++) {
        PIMAGE_SECTION_HEADER pSection = &pSectionHeader[i];

        // 计算节的实际大小（考虑对齐）
        ULONG virtualSize = pSection->Misc.VirtualSize;
        ULONG sizeOfRawData = pSection->SizeOfRawData;

        // 节的虚拟地址范围
        ULONG_PTR sectionStart = pSection->VirtualAddress;
        ULONG_PTR sectionEnd = sectionStart + virtualSize;

        Log("[PE] 检查节[%d]: %8s, VA=0x%x-0x%x, VSize=0x%x, FOA=0x%x, FSize=0x%x",
            i, pSection->Name, sectionStart, sectionEnd,
            virtualSize, pSection->PointerToRawData, sizeOfRawData);

        // 检查RVA是否在这个节的范围内
        if (Rva >= sectionStart && Rva < sectionEnd) {
            // 计算在节内的偏移
            ULONG offsetInSection = (ULONG)(Rva - sectionStart);

            // 检查偏移是否超出文件中的实际数据大小
            if (offsetInSection >= sizeOfRawData && sizeOfRawData > 0) {
                Log("[PE] 警告: RVA偏移0x%x超出节的文件数据大小0x%x",
                    offsetInSection, sizeOfRawData);
                // 对于BSS节或未初始化数据，返回0
                return 0;
            }

            // 计算FOA
            ULONG_PTR foa = pSection->PointerToRawData + offsetInSection;

            Log("[PE] RVA转换成功: RVA=0x%x -> 节%s -> FOA=0x%x (偏移=0x%x)",
                Rva, pSection->Name, foa, offsetInSection);

            return foa;
        }
    }

    Log("[PE] RVA转换失败: RVA=0x%x 不在任何节范围内", Rva);
    return 0;
}
