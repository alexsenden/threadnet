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
static const char UDP_PAYLOAD[] = "Hello OpenThread World!";
static void sendUdp(otInstance *aInstance);
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
void sendUdp(otInstance *aInstance)
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

    error = otMessageAppend(message, UDP_PAYLOAD, sizeof(UDP_PAYLOAD));
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

        otInstance *instance = esp_openthread_get_instance();
        sendUdp(instance);
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

void reportNetworkStatus()
{
    // otError err;

    otInstance *instance = esp_openthread_get_instance();

    otDeviceRole role = otThreadGetDeviceRole(instance);
    const otExtendedPanId *ext_pan_id = otThreadGetExtendedPanId(instance);
    // otLinkModeConfig ot_link_mode = otThreadGetLinkMode(instance);
    const otIp6Address *ot_rloc = otThreadGetRloc(instance);
    const otIp6Address *ot_mesh_local_eid = otThreadGetMeshLocalEid(instance);
    // const otMeshLocalPrefix *ot_mesh_local_prefix = otThreadGetMeshLocalPrefix(instance);
    const otIp6Address *ot_link_local_ipv6 = otThreadGetLinkLocalIp6Address(instance);
    const otIp6Address *ot_link_local_multicast = otThreadGetLinkLocalAllThreadNodesMulticastAddress(instance);
    const otIp6Address *ot_realm_local_multicast = otThreadGetRealmLocalAllThreadNodesMulticastAddress(instance);
    const char *ot_network_name = otThreadGetNetworkName(instance);
    const char *ot_domain_name = otThreadGetDomainName(instance);
    // uint32_t ot_key_seq_ctr = otThreadGetKeySequenceCounter(instance);
    // uint32_t ot_keyswitch_guardtime = otThreadGetKeySwitchGuardTime(instance);
    uint8_t ot_leader_id = otThreadGetLeaderRouterId(instance);
    uint8_t ot_leader_weight = otThreadGetLeaderWeight(instance);
    uint32_t ot_partition_id = otThreadGetPartitionId(instance);
    uint16_t ot_rloc_16 = otThreadGetRloc16(instance);
    // uint16_t ot_version = otThreadGetVersion();
    // const otIpCounters *ot_ipv6_counters = otThreadGetIp6Counters(instance);
    // const otMleCounters *ot_mle_counters = otThreadGetMleCounters(instance);

    // otIp6Address *ot_leader_rloc;
    // err = otThreadGetLeaderRloc(instance, &ot_leader_rloc);
    // if (err != OT_ERROR_NONE)
    // {
    //     ESP_LOGE(TAG, "Error getting leader RLOC");
    // }

    // otIp6Address *ot_service_rloc;
    // err = otThreadGetServiceAloc(instance, ot_service_rloc);

    // otNeighborInfo *ot_neighbor_info;
    // otNeighborInfoIterator *ot_iterator = OT_NEIGHBOR_INFO_ITERATOR_INIT;
    // err = otThreadGetNextNeighborInfo(instance, &ot_iterator, &ot_neighbor_info);
    // if (err != OT_ERROR_NONE)
    // {
    //     ESP_LOGE(TAG, "Error getting neighbor info");
    // }

    // otLeaderData *ot_leader_data;
    // err = otThreadGetLeaderData(instance, &ot_leader_data);
    // if (err != OT_ERROR_NONE)
    // {
    //     ESP_LOGE(TAG, "Error getting leader data");
    // }

    // otRouterInfo *ot_router_info;
    // err = otThreadGetParentInfo(instance, &ot_router_info);
    // if (err != OT_ERROR_NONE)
    // {
    //     ESP_LOGE(TAG, "Error getting parent info");
    // }

    // int8_t ot_parent_rssi;
    // err = otThreadGetParentAverageRssi(instance, &ot_parent_rssi);
    // if (err != OT_ERROR_NONE)
    // {
    //     ESP_LOGE(TAG, "Error getting parent average RSSI");
    // }

    // int8_t ot_last_rssi;
    // err = otThreadGetParentLastRssi(instance, &ot_last_rssi);
    // if (err != OT_ERROR_NONE)
    // {
    //     ESP_LOGE(TAG, "Error getting parent last RSSI");
    // }

    ESP_LOGI(TAG, "Network Status:");

    ESP_LOGI(TAG, "Role: %s", otThreadDeviceRoleToString(role));

    char ext_pan_id_str[17] = {0};
    for (int i = 0; i < 8; i++)
    {
        snprintf(&ext_pan_id_str[i * 2], 3, "%02X", ext_pan_id->m8[i]);
    }
    ESP_LOGI(TAG, "Extended PAN ID: 0x%s", ext_pan_id_str);

    char ip6_str[OT_IP6_ADDRESS_STRING_SIZE];
    otIp6AddressToString(ot_rloc, ip6_str, sizeof(ip6_str));
    ESP_LOGI(TAG, "RLOC: %s", ip6_str);

    otIp6AddressToString(ot_mesh_local_eid, ip6_str, sizeof(ip6_str));
    ESP_LOGI(TAG, "Mesh Local EID: %s", ip6_str);

    otIp6AddressToString(ot_link_local_ipv6, ip6_str, sizeof(ip6_str));
    ESP_LOGI(TAG, "Link Local IPv6: %s", ip6_str);

    otIp6AddressToString(ot_link_local_multicast, ip6_str, sizeof(ip6_str));
    ESP_LOGI(TAG, "Link Local Multicast: %s", ip6_str);

    otIp6AddressToString(ot_realm_local_multicast, ip6_str, sizeof(ip6_str));
    ESP_LOGI(TAG, "Realm Local Multicast: %s", ip6_str);

    ESP_LOGI(TAG, "Network Name: %s", ot_network_name);
    ESP_LOGI(TAG, "Domain Name: %s", ot_domain_name);
    ESP_LOGI(TAG, "Leader ID: %u", ot_leader_id);
    ESP_LOGI(TAG, "Leader Weight: %u", ot_leader_weight);
    ESP_LOGI(TAG, "Partition ID: %lu", ot_partition_id);
    ESP_LOGI(TAG, "RLOC16: %d", ot_rloc_16);

    ESP_LOGI(TAG, "End of Network Status");
}