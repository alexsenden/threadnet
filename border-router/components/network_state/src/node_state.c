#include "esp_log.h"
#include "esp_openthread.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "network_config.h"
#include "threadnet_app.h"

#define TAG "Node Status"

// Report self status to the network
static void reportNetworkStatus()
{
    threadnet_packet_t packet;
    otInstance *instance = esp_openthread_get_instance();

    // Get the device role
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
    otError err = otThreadGetParentInfo(instance, &parent_info);
    if (err != OT_ERROR_NONE)
    {
        ESP_LOGE(TAG, "An error occured getting parent info");
    }

    // Assemble the packet
    packet.parent_next_hop = parent_info.mNextHop;
    packet.parent_link_quality_in = parent_info.mLinkQualityIn;
    packet.parent_link_quality_out = parent_info.mLinkQualityOut;
    packet.parent_rloc16 = parent_info.mRloc16;
    packet.parent_age = parent_info.mAge;
    packet.parent_router_id = parent_info.mRouterId;
    packet.packet_success_rate = -1;

    // Update the store
    update_tracked_node(&packet);
}

// Report the node status to the network every STATUS_SEND_PERIOD_MS
static void report_node_status_worker()
{
    while (1)
    {
        vTaskDelay(STATUS_SEND_PERIOD_MS / portTICK_PERIOD_MS);
        reportNetworkStatus();
    }
}

// Start the node status task
void start_node_status_messages(void)
{
    xTaskCreate(report_node_status_worker, "report_node_status", 2048, NULL, 5, NULL);
    ESP_LOGI(TAG, "Broadcast network status task started");
}
