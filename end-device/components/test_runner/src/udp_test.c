#include <string.h>

#include "esp_log.h"
#include "esp_openthread.h"
#include "esp_openthread_lock.h"
#include "openthread/ip6.h"
#include "openthread/udp.h"

#include "network_state.h"
#include "network_config.h"

#define ADDR_SIZE 64
#define BUFFER_SIZE 1024

#define TAG "UDP Test Runner"

void send_udp_test_message(void){
    char body[BUFFER_SIZE];
    sprintf(body, "test message");

    char host_addr[ADDR_SIZE];
    get_network_host_ip(host_addr);

    otMessageInfo message_info;
    memset(&message_info, 0, sizeof(otMessageInfo));

    message_info.mHopLimit = 0xFF;
    // otIp6AddressFromString(host_addr, &message_info.mSockAddr);
    // otIp6AddressFromString(host_addr, &message_info.mPeerAddr);
    otIp6AddressFromString("ff03::1", &message_info.mSockAddr);
    otIp6AddressFromString("ff03::1", &message_info.mPeerAddr);
    message_info.mSockPort = UDP_TEST_PORT;
    message_info.mPeerPort = UDP_TEST_PORT;

    otInstance *aInstance = esp_openthread_get_instance();

    // Acquire OT Lock
    esp_openthread_lock_acquire(portMAX_DELAY);

    otMessage *net_state_message = otUdpNewMessage(aInstance, NULL);
    otMessageAppend(net_state_message, body, strlen(body));
    if(otUdpSendDatagram(aInstance, net_state_message, &message_info) != OT_ERROR_NONE) {
        ESP_LOGW(TAG, "Failed to send udp test message");
    }

    // Release OT Lock
    esp_openthread_lock_release();
}