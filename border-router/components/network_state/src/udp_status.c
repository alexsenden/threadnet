#include "esp_log.h"
#include "esp_openthread.h"
#include "esp_openthread_lock.h"
#include "freertos/FreeRTOS.h"
#include "openthread/udp.h"

#include "app_state.h"
#include "network_config.h"
#include "threadnet_app.h"

#define TAG "UDP Status Receiver"

static otUdpSocket foreign_status_udp_socket;

void handle_foreign_status_udp(void *aContext, otMessage *aMessage,
                      const otMessageInfo *aMessageInfo)
{
    char peerAddr[OT_IP6_ADDRESS_STRING_SIZE];
    otIp6AddressToString(&aMessageInfo->mPeerAddr, peerAddr, OT_IP6_ADDRESS_STRING_SIZE);

    ESP_LOGI(TAG, "Received UDP message from %s", peerAddr);

    // Short circuit if this message is of an inactive type
    if(get_transport_mode() != TRANSPORT_MODE_UDP) {
        return;
    }

    size_t dataLength = otMessageGetLength(aMessage);
    uint16_t offset = otMessageGetOffset(aMessage);
    uint8_t data[dataLength];
    otMessageRead(aMessage, offset, data, dataLength);

    threadnet_packet_t *packet = (threadnet_packet_t *)data;

    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    packet->last_time = tv_now.tv_sec;

    update_tracked_node(packet);
}

void init_udp_status_socket(otInstance *aInstance)
{
    otSockAddr listenSockAddr;

    memset(&foreign_status_udp_socket, 0, sizeof(foreign_status_udp_socket));
    memset(&listenSockAddr, 0, sizeof(listenSockAddr));

    listenSockAddr.mPort = UDP_STATUS_PORT;

    esp_openthread_lock_acquire(portMAX_DELAY);
    otUdpOpen(aInstance, &foreign_status_udp_socket, handle_foreign_status_udp, aInstance);
    otUdpBind(aInstance, &foreign_status_udp_socket, &listenSockAddr, OT_NETIF_THREAD);
    esp_openthread_lock_release();

    ESP_LOGI(TAG, "Listening for UDP messages on port %d", UDP_STATUS_PORT);
}
