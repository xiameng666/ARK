#pragma once
#include"mydef.h"


extern "C" {
    NTSTATUS EnumModuleEx(PMODULE_INFO ModuleBuffer, bool CountOnly, PULONG ModuleCount);
}
