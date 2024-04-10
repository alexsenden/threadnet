#include <string.h>
#include <unistd.h>

#include "esp_log.h"
#include "esp_openthread.h"
#include "esp_openthread_lock.h"
#include "openthread/udp.h"

#include "app_state.h"
#include "code_tools.h"
#include "network_config.h"
#include "node_state.h"

#define TAG "UDP Status"

static otUdpSocket node_state_socket;

// Receive an ACK packet from the UDP address
void handle_status_ack_udp(void *aContext, otMessage *aMessage,
                           const otMessageInfo *aMessageInfo)
{
    // Short circuit if this message is of an inactive type
    if (get_transport_mode() != TRANSPORT_MODE_UDP)
    {
        return;
    }

    // Get peer address
    char peerAddr[OT_IP6_ADDRESS_STRING_SIZE];
    otIp6AddressToString(&aMessageInfo->mPeerAddr, peerAddr, OT_IP6_ADDRESS_STRING_SIZE);

    // Read the data from the message
    size_t dataLength = otMessageGetLength(aMessage);
    uint16_t offset = otMessageGetOffset(aMessage);
    uint8_t data[dataLength];
    otMessageRead(aMessage, offset, data, dataLength);

    ack_packet_t *packet = (ack_packet_t *)data;

    ESP_LOGI(TAG, "Received UDP ACK message from %s with seq# %d", peerAddr, packet->sequence_num);
    handle_ack(packet);
}

/**
 * Initialize UDP socket
 */
void init_node_status_socket_udp(otInstance *aInstance)
{
    otSockAddr listenSockAddr;

    memset(&node_state_socket, 0, sizeof(node_state_socket));
    memset(&listenSockAddr, 0, sizeof(listenSockAddr));

    listenSockAddr.mPort = UDP_STATUS_PORT;

    otUdpOpen(aInstance, &node_state_socket, handle_status_ack_udp, aInstance);
    otUdpBind(aInstance, &node_state_socket, &listenSockAddr, OT_NETIF_THREAD);
}

/**
 * Send a UDP datagram
 */
void send_status_udp(otInstance *aInstance, void *buffer, uint32_t size)
{
    otError error = OT_ERROR_NONE;
    otMessage *message;
    otMessageInfo messageInfo;
    otIp6Address destinationAddr;
    char app_host_addr[OT_IP6_ADDRESS_STRING_SIZE];
    get_app_ip(app_host_addr);

    memset(&messageInfo, 0, sizeof(messageInfo));

    // Set the destination address
    otIp6AddressFromString(app_host_addr, &destinationAddr);
    messageInfo.mPeerAddr = destinationAddr;
    messageInfo.mPeerPort = UDP_STATUS_PORT;

    // Create a new message
    message = otUdpNewMessage(aInstance, NULL);
    otEXPECT_ACTION(message != NULL, error = OT_ERROR_NO_BUFS);

    error = otMessageAppend(message, buffer, size);
    otEXPECT(error == OT_ERROR_NONE);

    // Send the message
    ESP_LOGI(TAG, "Sending UDP message to %s port %d", app_host_addr, UDP_STATUS_PORT);
    error = otUdpSend(aInstance, &node_state_socket, message, &messageInfo);

exit:
    if (error != OT_ERROR_NONE && message != NULL)
    {
        // Cleanup the message
        otMessageFree(message);
    }
}
