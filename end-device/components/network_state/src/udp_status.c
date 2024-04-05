#include <string.h>
#include <unistd.h>

#include "esp_log.h"
#include "esp_openthread.h"
#include "esp_openthread_lock.h"
#include "openthread/udp.h"

#include "app_state.h"
#include "code_tools.h"
#include "network_config.h"

#define TAG "UDP Status"

static otUdpSocket app_state_socket;

/**
 * Initialize UDP socket
 */
void init_node_status_socket_udp(otInstance *aInstance)
{
    otSockAddr listenSockAddr;

    memset(&app_state_socket, 0, sizeof(app_state_socket));
    memset(&listenSockAddr, 0, sizeof(listenSockAddr));

    listenSockAddr.mPort = UDP_STATUS_PORT;

    otUdpBind(aInstance, &app_state_socket, &listenSockAddr, OT_NETIF_THREAD);
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

    otIp6AddressFromString(app_host_addr, &destinationAddr);
    messageInfo.mPeerAddr = destinationAddr;
    messageInfo.mPeerPort = UDP_STATUS_PORT;

    message = otUdpNewMessage(aInstance, NULL);
    otEXPECT_ACTION(message != NULL, error = OT_ERROR_NO_BUFS);

    error = otMessageAppend(message, buffer, size);
    otEXPECT(error == OT_ERROR_NONE);

    ESP_LOGI(TAG, "Sending UDP message to %s port %d", app_host_addr, UDP_STATUS_PORT);
    error = otUdpSend(aInstance, &app_state_socket, message, &messageInfo);

exit:
    if (error != OT_ERROR_NONE && message != NULL)
    {
        otMessageFree(message);
    }
}
