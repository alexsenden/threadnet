#pragma once

#include <unistd.h>
#include "esp_openthread.h"

#define TRANSPORT_MODE_TCP   1
#define TRANSPORT_MODE_UDP   2
#define TRANSPORT_MODE_MULTI 3

#define TCP_STATUS_PORT 8881
#define UDP_STATUS_PORT 8882
#define MULTICAST_STATUS_PORT 8883

#define TRANSPORT_MODE_BROADCAST_PORT 8888

#define STATUS_SEND_PERIOD_MS 10000

typedef __UINT8_TYPE__ transport_mode_t;

typedef struct threadnet_packet
{
    otDeviceRole role;
    char ot_mesh_local_eid[OT_IP6_ADDRESS_STRING_SIZE];
    char ot_link_local_ipv6[OT_IP6_ADDRESS_STRING_SIZE];
    uint8_t ot_leader_id;
    uint8_t ot_leader_weight;
    uint32_t ot_partition_id;
    uint16_t ot_rloc_16;
    int8_t parent_avg_rssi;
    int8_t parent_last_rssi;
    uint64_t last_time;
    uint8_t parent_next_hop;
    uint8_t parent_link_quality_in;
    uint8_t parent_link_quality_out;
    uint16_t parent_rloc16;
    uint8_t parent_age;
    uint8_t parent_router_id;
} threadnet_packet_t;
