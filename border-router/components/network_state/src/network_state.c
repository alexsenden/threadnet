#include <string.h>

#include "esp_log.h"
#include "esp_openthread.h"
#include "esp_openthread_lock.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "openthread/ip6.h"
#include "openthread/udp.h"

#include "network_state.h"
#include "network_config.h"

#define BROADCAST_PERIOD 60 * 1000 / portTICK_PERIOD_MS // 30 Seconds
#define BUFFER_SIZE 128

static const char *TAG = "Network State";

static int transport_mode = TRANSPORT_MODE_TCP;

void set_transport_mode(transport_mode_t mode) 
{
    switch (mode) {
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

static void set_broadcast_message_info(otMessageInfo *message_info) {
    memset(message_info, 0, sizeof(otMessageInfo));

    message_info->mHopLimit = 0xFF;
    otIp6AddressFromString("ff03::1", &message_info->mSockAddr);
    otIp6AddressFromString("ff03::1", &message_info->mPeerAddr);
    message_info->mSockPort = TRANSPORT_MODE_BROADCAST_PORT;
    message_info->mPeerPort = TRANSPORT_MODE_BROADCAST_PORT;
}

static void net_state_broadcast_task(void* aContext)
{   
    char buf[BUFFER_SIZE];

    otMessageInfo message_info;
    set_broadcast_message_info(&message_info);
    otInstance *aInstance = esp_openthread_get_instance();

    while(1) {
        vTaskDelay(BROADCAST_PERIOD);

        sprintf(buf, "transport_mode %d", transport_mode);

        ESP_LOGI(TAG, "Message: >%s<", buf);

        // Acquire OT Lock
        esp_openthread_lock_acquire(portMAX_DELAY);

        otMessage *net_state_message = otUdpNewMessage(aInstance, NULL);
        otMessageAppend(net_state_message, buf, strlen(buf));
        if(otUdpSendDatagram(aInstance, net_state_message, &message_info) != OT_ERROR_NONE) {
            ESP_LOGW(TAG, "Failed to send network state broadcast");
            }

        // Release OT Lock
        esp_openthread_lock_release();
    }
}

void start_net_state_broadcasts(void) 
{
    xTaskCreate(net_state_broadcast_task, "ot_network_state", 10240, xTaskGetCurrentTaskHandle(), 5, NULL);
}
