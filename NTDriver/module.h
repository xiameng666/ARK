#pragma once
#include"mydef.h"


extern "C" {
    //΢���
    NTSTATUS EnumModuleEx(PMODULE_INFO ModuleBuffer, bool CountOnly, PULONG ModuleCount);

    //����ģ��洢��g_ModuleBuffer ��Ҫ����ѭ����
    NTSTATUS EnumModule();

    //��g_ModuleBuffer������ ����ǰEnumModule����������Ч
    NTSTATUS FindModuleByAddress(PVOID Address, PCHAR ModulePath, PVOID* ImageBase, PULONG ImageSize);
}
