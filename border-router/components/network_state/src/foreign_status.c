#include "esp_openthread.h"

#include "multicast_status.h"
#include "network_config.h"
#include "udp_status.h"

void init_foreign_status_sockets(void)
{
    otInstance *ot_instance = esp_openthread_get_instance();

    init_udp_status_socket(ot_instance);
    init_multicast_status_socket(ot_instance);
    // init_tcp_status_socket(ot_instance);
    // Add other sockets here
}

void set_ack_packet(ack_packet_t *ack_packet, threadnet_packet_t *status)
{
    ack_packet->sequence_num = status->sequence_num;
    strcpy(ack_packet->ot_mesh_local_eid, status->ot_mesh_local_eid);
}
