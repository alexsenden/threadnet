#include "esp_log.h"
#include "esp_openthread.h"
#include "esp_openthread_lock.h"
#include "freertos/FreeRTOS.h"
#include "openthread/udp.h"

#include "app_state.h"
#include "code_tools.h"
#include "foreign_status.h"
#include "network_config.h"
#include "threadnet_app.h"

#define TAG "Multicast Status Receiver"

static otUdpSocket foreign_status_multicast_socket;
static otUdpSocket ack_multicast_socket;

static void send_status_ack_multicast(ack_packet_t *ack_packet)
{
    otError error = OT_ERROR_NONE;
    otMessage *message;
    otMessageInfo messageInfo;
    otIp6Address destinationAddr;
    otInstance *ot_instance = esp_openthread_get_instance();

    memset(&messageInfo, 0, sizeof(messageInfo));

    otIp6AddressFromString(MESH_MULTICAST_ADDR, &destinationAddr);
    messageInfo.mPeerAddr = destinationAddr;
    messageInfo.mPeerPort = MULTICAST_ACK_PORT;

    message = otUdpNewMessage(ot_instance, NULL);
    otEXPECT_ACTION(message != NULL, error = OT_ERROR_NO_BUFS);

    error = otMessageAppend(message, ack_packet, sizeof(ack_packet_t));
    otEXPECT(error == OT_ERROR_NONE);

    ESP_LOGI(TAG, "Sending multicast ack to %s port %d", MESH_MULTICAST_ADDR, MULTICAST_ACK_PORT);
    error = otUdpSend(ot_instance, &ack_multicast_socket, message, &messageInfo);

exit:
    if (error != OT_ERROR_NONE && message != NULL)
    {
        otMessageFree(message);
    }
}

void handle_foreign_status_multicast(void *aContext, otMessage *aMessage,
                      const otMessageInfo *aMessageInfo)
{
    // Short circuit if this message is of an inactive type
    if(get_transport_mode() != TRANSPORT_MODE_MULTI) {
        return;
    }

    char peerAddr[OT_IP6_ADDRESS_STRING_SIZE];
    otIp6AddressToString(&aMessageInfo->mPeerAddr, peerAddr, OT_IP6_ADDRESS_STRING_SIZE);

    ESP_LOGI(TAG, "Received multicast UDP message from %s", peerAddr);

    size_t dataLength = otMessageGetLength(aMessage);
    uint16_t offset = otMessageGetOffset(aMessage);
    uint8_t data[dataLength];
    otMessageRead(aMessage, offset, data, dataLength);

    threadnet_packet_t *packet = (threadnet_packet_t *)data;

    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    packet->last_time = tv_now.tv_sec;

    update_tracked_node(packet);

    ack_packet_t ack_packet;
    set_ack_packet(&ack_packet, packet);
    send_status_ack_multicast(&ack_packet);
}

void init_multicast_status_socket(otInstance *aInstance)
{
    otSockAddr listenSockAddr;
    otSockAddr ackSockAddr;

    memset(&foreign_status_multicast_socket, 0, sizeof(foreign_status_multicast_socket));
    memset(&listenSockAddr, 0, sizeof(listenSockAddr));
    memset(&ack_multicast_socket, 0, sizeof(ack_multicast_socket));
    memset(&ackSockAddr, 0, sizeof(ackSockAddr));

    listenSockAddr.mPort = MULTICAST_STATUS_PORT;
    ackSockAddr.mPort = MULTICAST_ACK_PORT;

    esp_openthread_lock_acquire(portMAX_DELAY);
    otUdpOpen(aInstance, &foreign_status_multicast_socket, handle_foreign_status_multicast, aInstance);
    otUdpBind(aInstance, &foreign_status_multicast_socket, &listenSockAddr, OT_NETIF_THREAD);

    otUdpBind(aInstance, &ack_multicast_socket, &ackSockAddr, OT_NETIF_THREAD);
    esp_openthread_lock_release();

    ESP_LOGI(TAG, "Listening for UDP multicast statuses on port %d", MULTICAST_STATUS_PORT);
}
