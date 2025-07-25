#pragma once
#include "driver.h"

// 枚举所有回调函数
NTSTATUS EnumCallbacks(PCALLBACK_INFO callbackBuffer, CALLBACK_TYPE type, PULONG callbackCount);
NTSTATUS DeleteCallback(CALLBACK_TYPE type, ULONG index);