#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_netif_types.h"
#include "esp_openthread.h"
#include "esp_openthread_cli.h"
#include "esp_openthread_lock.h"
#include "esp_openthread_netif_glue.h"
#include "esp_openthread_types.h"
#include "esp_ot_config.h"
#include "esp_spiffs.h"
#include "esp_rcp_update.h"
#include "esp_vfs_eventfd.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hal/uart_types.h"
#include "nvs_flash.h"
#include "openthread/cli.h"
#include "openthread/instance.h"
#include "openthread/logging.h"
#include "openthread/tasklet.h"
#include "openthread/udp.h"
#include "openthread/message.h"

#include "thread_init.h"

#define TAG "thread_worker"

#define UDP_PORT 1212

void reportNetworkStatus();
static void initUdp(otInstance *aInstance);
void handleUdpReceive(void *aContext, otMessage *aMessage,
                      const otMessageInfo *aMessageInfo);

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

    otUdpOpen(aInstance, &sUdpSocket, handleUdpReceive, aInstance);
    otUdpBind(aInstance, &sUdpSocket, &listenSockAddr, OT_NETIF_THREAD);
}

static esp_err_t init_spiffs(void)
{
    esp_vfs_spiffs_conf_t rcp_fw_conf = {
        .base_path = "/rcp_fw", .partition_label = "rcp_fw", .max_files = 10, .format_if_mount_failed = false};
    ESP_ERROR_CHECK(esp_vfs_spiffs_register(&rcp_fw_conf));

    return ESP_OK;
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

    esp_openthread_lock_acquire(portMAX_DELAY);

    // Initialize the esp_netif bindings
    esp_netif_t *openthread_netif;
    openthread_netif = init_openthread_netif(&config);
    esp_netif_set_default_netif(openthread_netif);

    otInstance *instance = esp_openthread_get_instance();
    initUdp(instance);

    ESP_ERROR_CHECK(esp_openthread_auto_start(NULL));

    esp_openthread_lock_release();
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
    ESP_ERROR_CHECK(init_spiffs());

    esp_rcp_update_config_t rcp_update_config = ESP_OPENTHREAD_RCP_UPDATE_CONFIG();
    ESP_ERROR_CHECK(esp_rcp_update_init(&rcp_update_config));

    xTaskCreate(ot_task_worker, "ot_main_worker", 10240, xTaskGetCurrentTaskHandle(), 5, NULL);

    xTaskCreate(broadcastNetworkStatus, "broadcast_network_status", 2048, NULL, 5, NULL);
    ESP_LOGI(TAG, "Broadcast network status task started");
}

uint8_t (*update_cb)(threadnet_packet_t *);

void register_update_cb(uint8_t (*cb)(threadnet_packet_t *))
{
    update_cb = cb;
}

void handleUdpReceive(void *aContext, otMessage *aMessage,
                      const otMessageInfo *aMessageInfo)
{
    if (update_cb == NULL)
    {
        ESP_LOGE(TAG, "No callback set for handling UDP messages");
        return;
    }

    char peerAddr[OT_IP6_ADDRESS_STRING_SIZE];
    otIp6AddressToString(&aMessageInfo->mPeerAddr, peerAddr, OT_IP6_ADDRESS_STRING_SIZE);

    ESP_LOGI(TAG, "Received UDP message from %s", peerAddr);

    size_t dataLength = otMessageGetLength(aMessage);
    uint16_t offset = otMessageGetOffset(aMessage);
    uint8_t data[dataLength];
    otMessageRead(aMessage, offset, data, dataLength);

    threadnet_packet_t *packet = (threadnet_packet_t *)data;

    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    packet->last_time = tv_now.tv_sec;

    if (update_cb(packet))
    {
        ESP_LOGI(TAG, "New node discovered");
    }
}

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

    if (update_cb == NULL)
    {
        ESP_LOGE(TAG, "No callback set for handling UDP messages");
        return;
    }
    else
    {
        if (update_cb(&packet))
        {
            ESP_LOGI(TAG, "New node discovered");
        }
    }
}