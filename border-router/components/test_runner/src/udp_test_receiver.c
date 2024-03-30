#include "esp_log.h"
#include "esp_openthread.h"
#include "esp_openthread_lock.h"
#include "freertos/FreeRTOS.h"
#include "openthread/udp.h"

#include "test_runner.h"
#include "network_config.h"
#include "network_state.h"

#define TAG "UDP Test Receiver"

static otUdpReceiver *udp_test_receiver;

static bool udp_test_message_handler(void *aContext, const otMessage *aMessage, const otMessageInfo *aMessageInfo)
{
    ESP_LOGI(TAG, "UDP message received");
    if(aMessageInfo->mSockPort != UDP_TEST_PORT || get_transport_mode() != TRANSPORT_MODE_UDP) {
        ESP_LOGI(TAG, "UDP message discarded. Reasons: port(%d), mode(%d).", aMessageInfo->mSockPort != UDP_TEST_PORT, get_transport_mode() != TRANSPORT_MODE_UDP);
        return false;
    }

    handle_message(aMessage, aMessageInfo);

    return true;
}

void init_udp_test_listener(void)
{
    udp_test_receiver = malloc(sizeof(otUdpReceiver));
    udp_test_receiver->mHandler = udp_test_message_handler;

    otInstance *aInstance = esp_openthread_get_instance();

    // Acquire OT Lock
    esp_openthread_lock_acquire(portMAX_DELAY);

    ESP_LOGI(TAG, "Assigning UDP test message handler");
    if(otUdpAddReceiver(aInstance, udp_test_receiver) != OT_ERROR_NONE){
        ESP_LOGW(TAG, "Could not add UDP test receiver");
    }

    // Release OT Lock
    esp_openthread_lock_release();
}