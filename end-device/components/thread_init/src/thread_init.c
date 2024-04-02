#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "esp_err.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_netif_types.h"
#include "esp_openthread.h"
#include "esp_openthread_lock.h"
#include "esp_openthread_netif_glue.h"
#include "esp_openthread_types.h"
#include "esp_ot_config.h"
#include "esp_vfs_eventfd.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "openthread/cli.h"
#include "openthread/instance.h"
#include "openthread/logging.h"
#include "openthread/tasklet.h"
#include "openthread/udp.h"

#include "code_tools.h"

#define TAG "thread_worker"

void reportNetworkStatus();
void handleNetifStateChanged(uint32_t aFlags, void *aContext);

#define UDP_PORT 1212

static const char UDP_DEST_ADDR[] = "ff03::1";
static void sendUdp(otInstance *aInstance, void *buffer, uint32_t size);
static otUdpSocket sUdpSocket;

/**
 * Initialize UDP socket
 */
void initUdp(otInstance *aInstance)
{
    otSockAddr listenSockAddr;

    memset(&sUdpSocket, 0, sizeof(sUdpSocket));
    memset(&listenSockAddr, 0, sizeof(listenSockAddr));

    listenSockAddr.mPort = UDP_PORT;

    otUdpBind(aInstance, &sUdpSocket, &listenSockAddr, OT_NETIF_THREAD);
}

/**
 * Send a UDP datagram
 */
void sendUdp(otInstance *aInstance, void *buffer, uint32_t size)
{
    otError error = OT_ERROR_NONE;
    otMessage *message;
    otMessageInfo messageInfo;
    otIp6Address destinationAddr;

    memset(&messageInfo, 0, sizeof(messageInfo));

    otIp6AddressFromString(UDP_DEST_ADDR, &destinationAddr);
    messageInfo.mPeerAddr = destinationAddr;
    messageInfo.mPeerPort = UDP_PORT;

    message = otUdpNewMessage(aInstance, NULL);
    otEXPECT_ACTION(message != NULL, error = OT_ERROR_NO_BUFS);

    error = otMessageAppend(message, buffer, size);
    otEXPECT(error == OT_ERROR_NONE);

    ESP_LOGI(TAG, "Sending UDP message to %s port %d", UDP_DEST_ADDR, UDP_PORT);
    error = otUdpSend(aInstance, &sUdpSocket, message, &messageInfo);

exit:
    if (error != OT_ERROR_NONE && message != NULL)
    {
        otMessageFree(message);
    }
}

static esp_netif_t *init_openthread_netif(const esp_openthread_platform_config_t *config)
{
    esp_netif_config_t cfg = ESP_NETIF_DEFAULT_OPENTHREAD();
    esp_netif_t *netif = esp_netif_new(&cfg);
    assert(netif != NULL);
    ESP_ERROR_CHECK(esp_netif_attach(netif, esp_openthread_netif_glue_init(config)));

    return netif;
}

static void ot_task_worker(void *aContext)
{
    esp_openthread_platform_config_t config = {
        .radio_config = ESP_OPENTHREAD_DEFAULT_RADIO_CONFIG(),
        .host_config = ESP_OPENTHREAD_DEFAULT_HOST_CONFIG(),
        .port_config = ESP_OPENTHREAD_DEFAULT_PORT_CONFIG(),
    };

    // Initialize the OpenThread stack
    ESP_ERROR_CHECK(esp_openthread_init(&config));

    esp_netif_t *openthread_netif;
    // Initialize the esp_netif bindings
    openthread_netif = init_openthread_netif(&config);
    esp_netif_set_default_netif(openthread_netif);

    esp_openthread_lock_init();

    otInstance *instance = esp_openthread_get_instance();
    initUdp(instance);
    ESP_ERROR_CHECK(otSetStateChangedCallback(instance, handleNetifStateChanged, instance));

    ESP_ERROR_CHECK(esp_openthread_auto_start(NULL));
    esp_openthread_launch_mainloop();

    // Clean up
    esp_openthread_netif_glue_deinit();
    esp_netif_destroy(openthread_netif);

    esp_vfs_eventfd_unregister();
    vTaskDelete(NULL);
}

void broadcastNetworkStatus()
{
    while (1)
    {
        vTaskDelay(10000 / portTICK_PERIOD_MS);
        reportNetworkStatus();
    }
}

void start_thread_network(void)
{
    xTaskCreate(ot_task_worker, "ot_main_worker", 10240, xTaskGetCurrentTaskHandle(), 5, NULL);
    ESP_LOGI(TAG, "Thread network started");

    xTaskCreate(broadcastNetworkStatus, "broadcast_network_status", 2048, NULL, 5, NULL);
    ESP_LOGI(TAG, "Broadcast network status task started");
}

void handleNetifStateChanged(uint32_t aFlags, void *aContext)
{
    // reportNetworkStatus();
}

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

void reportNetworkStatus()
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
    otThreadGetParentInfo(instance, &parent_info);

    packet.parent_next_hop = parent_info.mNextHop;
    packet.parent_link_quality_in = parent_info.mLinkQualityIn;
    packet.parent_link_quality_out = parent_info.mLinkQualityOut;
    packet.parent_rloc16 = parent_info.mRloc16;
    packet.parent_age = parent_info.mAge;
    packet.parent_router_id = parent_info.mRouterId;

    sendUdp(instance, &packet, sizeof(threadnet_packet_t));
}