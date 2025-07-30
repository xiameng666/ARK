#define _WINSOCK_DEPRECATED_NO_WARNINGS
// Need to link with Iphlpapi.lib and Ws2_32.lib

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <psapi.h>  // 用于获取进程路径

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

#pragma comment(lib, "psapi.lib")
