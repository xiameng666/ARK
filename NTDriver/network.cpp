#include "network.h"


NTSTATUS EnumNetworkPort(PNETWORK_PORT_INFO PortBuffer, PULONG PortCount) {
    UNREFERENCED_PARAMETER(PortBuffer);
    *PortCount = 0;

    Log("[XM] EnumNetworkPort: Starting network port enumeration");

    //保留 后续可能需要R0帮助

    Log("[XM] EnumNetworkPort: Total ports found: %lu", *PortCount);
    return STATUS_SUCCESS;
}
