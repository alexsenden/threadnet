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

#define TAG "UDP Status Receiver"

static otUdpSocket foreign_status_udp_socket;

static void send_status_ack_udp(ack_packet_t *ack_packet)
{
    otError error = OT_ERROR_NONE;
    otMessage *message;
    otMessageInfo messageInfo;
    otIp6Address destinationAddr;
    otInstance *ot_instance = esp_openthread_get_instance();

    memset(&messageInfo, 0, sizeof(messageInfo));

    otIp6AddressFromString(ack_packet->ot_mesh_local_eid, &destinationAddr);
    messageInfo.mPeerAddr = destinationAddr;
    messageInfo.mPeerPort = UDP_STATUS_PORT;

    message = otUdpNewMessage(ot_instance, NULL);
    otEXPECT_ACTION(message != NULL, error = OT_ERROR_NO_BUFS);

    error = otMessageAppend(message, ack_packet, sizeof(ack_packet_t));
    otEXPECT(error == OT_ERROR_NONE);

    ESP_LOGI(TAG, "Sending UDP ack to %s port %d", ack_packet->ot_mesh_local_eid, UDP_STATUS_PORT);
    error = otUdpSend(ot_instance, &foreign_status_udp_socket, message, &messageInfo);

exit:
    if (error != OT_ERROR_NONE && message != NULL)
    {
        otMessageFree(message);
    }
}

void handle_foreign_status_udp(void *aContext, otMessage *aMessage,
                      const otMessageInfo *aMessageInfo)
{
    // Short circuit if this message is of an inactive type
    if(get_transport_mode() != TRANSPORT_MODE_UDP) {
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

    update_tracked_node(packet);

    ack_packet_t ack_packet;
    set_ack_packet(&ack_packet, packet);
    send_status_ack_udp(&ack_packet);
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