#include <string.h>

#include "esp_log.h"
#include "esp_openthread.h"
#include "esp_openthread_lock.h"
#include "freertos/FreeRTOS.h"
#include "openthread/udp.h"

#include "test_runner.h"
#include "network_config.h"
#include "network_state.h"

static otUdpReceiver *udp_test_start_receiver;

static bool test_start_message_handler(void *aContext, const otMessage *aMessage, const otMessageInfo *aMessageInfo)
{
    if(aMessageInfo->mSockPort != TEST_START_PORT) {
        return false;
    }

    char net_host_id[64];
    get_network_host_ip(net_host_id);
    if(!strcmp(net_host_id, "")){
        return true;
    }

    start_test();

    return true;
}

void init_test_start_listener(void)
{
    udp_test_start_receiver = malloc(sizeof(otUdpReceiver));
    udp_test_start_receiver->mHandler = test_start_message_handler;

    otInstance *aInstance = esp_openthread_get_instance();

    // Acquire OT Lock
    esp_openthread_lock_acquire(portMAX_DELAY);

    otUdpAddReceiver(aInstance, udp_test_start_receiver);

    // Release OT Lock
    esp_openthread_lock_release();
}
