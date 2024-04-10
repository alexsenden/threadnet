#include <string.h>

#include "esp_log.h"
#include "esp_openthread.h"
#include "esp_openthread_lock.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "openthread/ip6.h"
#include "openthread/udp.h"

#include "app_state.h"
#include "network_config.h"

#define BUFFER_SIZE 256

static const char *TAG = "Network State";

static transport_mode_t transport_mode = TRANSPORT_MODE_MULTI;

// Get the transport mode
transport_mode_t get_transport_mode()
{
    return transport_mode;
}

// Set the transport mode
void set_transport_mode(transport_mode_t mode)
{
    switch (mode)
    {
    case TRANSPORT_MODE_TCP:
    case TRANSPORT_MODE_UDP:
    case TRANSPORT_MODE_MULTI:
        transport_mode = mode;
        ESP_LOGI(TAG, "Network state updated to %d", transport_mode);
        break;
    default:
        ESP_LOGW(TAG, "Invalid network state update attempt: %d", transport_mode);
    }
}

// Set the broadcast message info
static void set_broadcast_message_info(otMessageInfo *message_info)
{
    // clear the message info
    memset(message_info, 0, sizeof(otMessageInfo));

    // set the message info
    message_info->mHopLimit = 0xFF;
    otIp6AddressFromString("ff03::1", &message_info->mSockAddr);
    otIp6AddressFromString("ff03::1", &message_info->mPeerAddr);
    message_info->mSockPort = TRANSPORT_MODE_BROADCAST_PORT;
    message_info->mPeerPort = TRANSPORT_MODE_BROADCAST_PORT;
}

// Broadcast the network state every STATUS_SEND_PERIOD_MS
static void net_state_broadcast_task(void *aContext)
{
    char buf[BUFFER_SIZE];
    char mesh_local_eid[OT_IP6_ADDRESS_STRING_SIZE];

    otMessageInfo message_info;
    set_broadcast_message_info(&message_info);
    otInstance *aInstance = esp_openthread_get_instance();

    // every STATUS_SEND_PERIOD_MS, broadcast the network state
    while (1)
    {
        // Wait for the next broadcast period
        vTaskDelay(STATUS_SEND_PERIOD_MS / portTICK_PERIOD_MS);

        // get the mesh local EID
        otIp6AddressToString(otThreadGetMeshLocalEid(aInstance), mesh_local_eid, OT_IP6_ADDRESS_STRING_SIZE);
        sprintf(buf, "transport_mode %d mesh_local_eid %s", transport_mode, mesh_local_eid);

        ESP_LOGI(TAG, "Broadcasting app state with transport mode %d", transport_mode);

        // Acquire OT Lock
        esp_openthread_lock_acquire(portMAX_DELAY);

        // Send the network state broadcast
        otMessage *net_state_message = otUdpNewMessage(aInstance, NULL);
        otMessageAppend(net_state_message, buf, strlen(buf));
        if (otUdpSendDatagram(aInstance, net_state_message, &message_info) != OT_ERROR_NONE)
        {
            ESP_LOGW(TAG, "Failed to send network state broadcast");
        }

        // Release OT Lock
        esp_openthread_lock_release();
    }
}

void start_net_state_broadcasts(void)
{
    // start the network state broadcast task
    xTaskCreate(net_state_broadcast_task, "ot_network_state", 10240, xTaskGetCurrentTaskHandle(), 5, NULL);
}
