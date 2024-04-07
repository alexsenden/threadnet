#include "esp_log.h"
#include "esp_openthread.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "network_config.h"
#include "threadnet_app.h"

#define TAG "Node Status"

static void reportNetworkStatus()
{
    threadnet_packet_t packet;
    otInstance *instance = esp_openthread_get_instance();

    otDeviceRole role = otThreadGetDeviceRole(instance);
    packet.role = role;

    uint8_t ot_leader_id = otThreadGetLeaderRouterId(instance);
    packet.ot_leader_id = ot_leader_id;

    uint8_t ot_leader_weight = otThreadGetLeaderWeight(instance);
    packet.ot_leader_weight = ot_leader_weight;

    uint32_t ot_partition_id = otThreadGetPartitionId(instance);
    packet.ot_partition_id = ot_partition_id;

    uint16_t ot_rloc_16 = otThreadGetRloc16(instance);
    packet.ot_rloc_16 = ot_rloc_16;

    const otIp6Address *ot_mesh_local_eid = otThreadGetMeshLocalEid(instance);
    otIp6AddressToString(ot_mesh_local_eid, packet.ot_mesh_local_eid, OT_IP6_ADDRESS_STRING_SIZE);

    const otIp6Address *ot_link_local_ipv6 = otThreadGetLinkLocalIp6Address(instance);
    otIp6AddressToString(ot_link_local_ipv6, packet.ot_link_local_ipv6, OT_IP6_ADDRESS_STRING_SIZE);

    otThreadGetParentAverageRssi(instance, &packet.parent_avg_rssi);
    otThreadGetParentLastRssi(instance, &packet.parent_last_rssi);

    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    packet.last_time = tv_now.tv_sec;

    otRouterInfo parent_info;
    otError err = otThreadGetParentInfo(instance, &parent_info);
    if (err != OT_ERROR_NONE)
    {
        ESP_LOGE(TAG, "An error occured getting parent info");
    }

    packet.parent_next_hop = parent_info.mNextHop;
    packet.parent_link_quality_in = parent_info.mLinkQualityIn;
    packet.parent_link_quality_out = parent_info.mLinkQualityOut;
    packet.parent_rloc16 = parent_info.mRloc16;
    packet.parent_age = parent_info.mAge;
    packet.parent_router_id = parent_info.mRouterId;
    packet.packet_success_rate = -1;

    update_tracked_node(&packet);
}

static void report_node_status_worker()
{
    while (1)
    {
        vTaskDelay(STATUS_SEND_PERIOD_MS / portTICK_PERIOD_MS);
        reportNetworkStatus();
    }
}

void start_node_status_messages(void)
{
    xTaskCreate(report_node_status_worker, "report_node_status", 2048, NULL, 5, NULL);
    ESP_LOGI(TAG, "Broadcast network status task started");
}
