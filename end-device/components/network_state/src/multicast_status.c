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

#define TAG "Multicast Status"

static otUdpSocket ack_socket;
static otUdpSocket node_state_socket;

static char mesh_local_eid[OT_IP6_ADDRESS_STRING_SIZE];

void handle_status_ack_multicast(void *aContext, otMessage *aMessage,
                      const otMessageInfo *aMessageInfo)
{
    // Short circuit if this message is of an inactive type
    if(get_transport_mode() != TRANSPORT_MODE_MULTI) {
        return;
    }

    char peerAddr[OT_IP6_ADDRESS_STRING_SIZE];
    otIp6AddressToString(&aMessageInfo->mPeerAddr, peerAddr, OT_IP6_ADDRESS_STRING_SIZE);    

    size_t dataLength = otMessageGetLength(aMessage);
    uint16_t offset = otMessageGetOffset(aMessage);
    uint8_t data[dataLength];
    otMessageRead(aMessage, offset, data, dataLength);

    ack_packet_t *packet = (ack_packet_t *)data;

    if(!strcmp(packet->ot_mesh_local_eid, mesh_local_eid)){
        ESP_LOGI(TAG, "Received multicast ACK message from %s with seq# %d", peerAddr, packet->sequence_num);
        handle_ack(packet);
    }
}

/**
 * Initialize UDP socket
 */
void init_node_status_socket_multicast(otInstance *aInstance)
{
    otSockAddr status_send_sock_addr;
    otSockAddr listenSockAddr;

    memset(&node_state_socket, 0, sizeof(node_state_socket));
    memset(&listenSockAddr, 0, sizeof(listenSockAddr));
    memset(&ack_socket, 0, sizeof(ack_socket));
    memset(&status_send_sock_addr, 0, sizeof(status_send_sock_addr));

    status_send_sock_addr.mPort = MULTICAST_STATUS_PORT;
    listenSockAddr.mPort = MULTICAST_ACK_PORT;

    const otIp6Address *ot_mesh_local_eid = otThreadGetMeshLocalEid(aInstance);
    otIp6AddressToString(ot_mesh_local_eid, mesh_local_eid, OT_IP6_ADDRESS_STRING_SIZE);

    otUdpOpen(aInstance, &ack_socket, handle_status_ack_multicast, aInstance);
    otUdpBind(aInstance, &ack_socket, &listenSockAddr, OT_NETIF_THREAD);
    otUdpBind(aInstance, &node_state_socket, &status_send_sock_addr, OT_NETIF_THREAD);
}

/**
 * Send a UDP datagram
 */
void send_status_multicast(otInstance *aInstance, void *buffer, uint32_t size)
{
    otError error = OT_ERROR_NONE;
    otMessage *message;
    otMessageInfo messageInfo;
    otIp6Address destinationAddr;

    memset(&messageInfo, 0, sizeof(messageInfo));

    otIp6AddressFromString(MESH_MULTICAST_ADDR, &destinationAddr);
    messageInfo.mPeerAddr = destinationAddr;
    messageInfo.mPeerPort = MULTICAST_STATUS_PORT;

    message = otUdpNewMessage(aInstance, NULL);
    otEXPECT_ACTION(message != NULL, error = OT_ERROR_NO_BUFS);

    error = otMessageAppend(message, buffer, size);
    otEXPECT(error == OT_ERROR_NONE);

    ESP_LOGI(TAG, "Sending multicast UDP message to %s port %d", MESH_MULTICAST_ADDR, MULTICAST_STATUS_PORT);
    error = otUdpSend(aInstance, &node_state_socket, message, &messageInfo);

exit:
    if (error != OT_ERROR_NONE && message != NULL)
    {
        otMessageFree(message);
    }
}
