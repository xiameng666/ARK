#pragma once
#include"mydef.h"


extern "C" {
    //微软的
    NTSTATUS EnumModuleEx(PMODULE_INFO ModuleBuffer, bool CountOnly, PULONG ModuleCount);

    //遍历模块存储到g_ModuleBuffer 不要放在循环内
    NTSTATUS EnumModule();

    //从g_ModuleBuffer拿数据 调用前EnumModule保持数据有效
    NTSTATUS FindModuleByAddress(PVOID Address, PCHAR ModulePath, PVOID* ImageBase, PULONG ImageSize);
}
