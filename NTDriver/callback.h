#pragma once
#include "driver.h"

// ö�����лص�����
NTSTATUS EnumCallbacks(PCALLBACK_INFO callbackBuffer, CALLBACK_TYPE type, PULONG callbackCount);
NTSTATUS DeleteCallback(CALLBACK_TYPE type, ULONG index);