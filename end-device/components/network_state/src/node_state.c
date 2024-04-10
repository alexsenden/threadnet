#include <string.h>
#include <unistd.h>

#include "esp_log.h"
#include "esp_openthread.h"
#include "esp_openthread_lock.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "openthread/udp.h"

#include "app_state.h"
#include "code_tools.h"
#include "multicast_status.h"
#include "network_config.h"
#include "udp_status.h"
#include "utils.h"

#define TAG "Node Status"
#define PACKET_MEMORY_SIZE 120000 / STATUS_SEND_PERIOD_MS

typedef struct packet_metrics
{
    uint32_t rtt;
    uint64_t send_time_millis;
} packet_metrics_t;

static packet_metrics_t packet_history[PACKET_MEMORY_SIZE];
static uint8_t next_seq_num = 0;

// Clear the packet history
void clear_history(void)
{
    for (int i = 0; i < PACKET_MEMORY_SIZE; i++)
    {
        packet_history[i].rtt = 0;
        packet_history[i].send_time_millis = 0;
    }
}

// Handle an incoming ACK packet
void handle_ack(ack_packet_t *ack_packet)
{
    if (ack_packet->sequence_num < PACKET_MEMORY_SIZE)
    {
        packet_metrics_t *metrics = &packet_history[ack_packet->sequence_num];
        metrics->rtt = current_time_millis() - metrics->send_time_millis;
    }
}

// Send a status packet
static void send_status(otInstance *aInstance, void *buffer, uint32_t size)
{
    switch (get_transport_mode())
    {
    case TRANSPORT_MODE_TCP:
        break;
    case TRANSPORT_MODE_MULTI:
        send_status_multicast(aInstance, buffer, size);
        break;
    case TRANSPORT_MODE_UDP:
        send_status_udp(aInstance, buffer, size);
    default:
        break;
    }
}

// Report the node status
static void report_node_status()
{
    // Create a packet with the node status
    threadnet_packet_t packet;
    otInstance *instance = esp_openthread_get_instance();

    // Get role
    otDeviceRole role = otThreadGetDeviceRole(instance);
    packet.role = role;

    // Get the leader id
    uint8_t ot_leader_id = otThreadGetLeaderRouterId(instance);
    packet.ot_leader_id = ot_leader_id;

    // Get the leader weight
    uint8_t ot_leader_weight = otThreadGetLeaderWeight(instance);
    packet.ot_leader_weight = ot_leader_weight;

    // Get the partition id
    uint32_t ot_partition_id = otThreadGetPartitionId(instance);
    packet.ot_partition_id = ot_partition_id;

    // Get the RLOC16
    uint16_t ot_rloc_16 = otThreadGetRloc16(instance);
    packet.ot_rloc_16 = ot_rloc_16;

    // Get the mesh local EID
    const otIp6Address *ot_mesh_local_eid = otThreadGetMeshLocalEid(instance);
    otIp6AddressToString(ot_mesh_local_eid, packet.ot_mesh_local_eid, OT_IP6_ADDRESS_STRING_SIZE);

    // Get link local IPv6 address
    const otIp6Address *ot_link_local_ipv6 = otThreadGetLinkLocalIp6Address(instance);
    otIp6AddressToString(ot_link_local_ipv6, packet.ot_link_local_ipv6, OT_IP6_ADDRESS_STRING_SIZE);

    // Get the parent average and last RSSI
    otThreadGetParentAverageRssi(instance, &packet.parent_avg_rssi);
    otThreadGetParentLastRssi(instance, &packet.parent_last_rssi);

    // Get the time
    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    packet.last_time = tv_now.tv_sec;

    // Get the parent info
    otRouterInfo parent_info;
    otThreadGetParentInfo(instance, &parent_info);

    // Fill in the parent info
    packet.parent_next_hop = parent_info.mNextHop;
    packet.parent_link_quality_in = parent_info.mLinkQualityIn;
    packet.parent_link_quality_out = parent_info.mLinkQualityOut;
    packet.parent_rloc16 = parent_info.mRloc16;
    packet.parent_age = parent_info.mAge;
    packet.parent_router_id = parent_info.mRouterId;

    // Calculate the average RTT and packet success rate
    int lost_packets = 0;
    int sent_packets = 0;
    uint32_t rtt_sum = 0;
    for (int i = 0; i < PACKET_MEMORY_SIZE; i++)
    {
        if (packet_history[i].send_time_millis != 0)
        {
            sent_packets++;
            if (packet_history[i].rtt == 0)
            {
                lost_packets++;
            }
            else
            {
                rtt_sum += packet_history[i].rtt;
            }
        }
    }
    int successful_packets = (sent_packets - lost_packets);
    packet.avg_rtt_millis = (successful_packets != 0) ? rtt_sum / successful_packets : 0;
    packet.packet_success_rate = (sent_packets != 0) ? ((float)(sent_packets - lost_packets)) / sent_packets : 0;

    packet_history[next_seq_num].rtt = 0;
    packet_history[next_seq_num].send_time_millis = current_time_millis();

    packet.sequence_num = next_seq_num++;
    next_seq_num %= PACKET_MEMORY_SIZE;

    // Send the packet
    send_status(instance, &packet, sizeof(threadnet_packet_t));
}

// Send node status every STATUS_SEND_PERIOD_MS
void send_node_status()
{
    while (1)
    {
        vTaskDelay(STATUS_SEND_PERIOD_MS / portTICK_PERIOD_MS);
        report_node_status();
    }
}

// Initialize the node status sockets
static void init_node_status_sockets(otInstance *aInstance)
{
    init_node_status_socket_udp(aInstance);
    init_node_status_socket_multicast(aInstance);
    // Add other socket inits here
}

// Start the node status messages
void start_node_status_messages(void)
{
    otInstance *ot_instance = esp_openthread_get_instance();

    // Aquiring the OT lock
    esp_openthread_lock_acquire(portMAX_DELAY);

    // Initialize the node status sockets
    init_node_status_sockets(ot_instance);

    // Release the OT lock
    esp_openthread_lock_release();

    // Start the send node status task
    xTaskCreate(send_node_status, "send_node_status", 2048, NULL, 5, NULL);
    ESP_LOGI(TAG, "Send node status task started");
}
