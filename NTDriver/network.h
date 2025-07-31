#pragma once
#include "mydef.h"

extern "C" {

    NTSTATUS EnumNetworkPort(PNETWORK_PORT_INFO PortBuffer, PULONG PortCount);//Ã¶¾ÙÍøÂç¶Ë¿Ú
    NTSTATUS EnumTcpPorts(PNETWORK_PORT_INFO PortBuffer, PULONG PortCount);
    NTSTATUS EnumUdpPorts(PNETWORK_PORT_INFO PortBuffer, PULONG PortCount);
}
