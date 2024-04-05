#include <unistd.h>

#include "esp_log.h"
#include "esp_openthread.h"
#include "esp_openthread_lock.h"
#include "freertos/FreeRTOS.h"
#include "openthread/udp.h"

#include "network_config.h"
#include "led.h"

#define TAG "Net State"

static transport_mode_t transport_mode = 0;
static char app_ip[OT_IP6_ADDRESS_STRING_SIZE];

static otUdpSocket app_state_socket;

transport_mode_t get_transport_mode(void)
{
    return transport_mode;
}

static void set_transport_mode_led(int transport_mode)
{
    switch (transport_mode)
    {
    case TRANSPORT_MODE_UDP:
        set_led_color(32, 0, 0);
        break;
    case TRANSPORT_MODE_TCP:
        set_led_color(0, 32, 0);
        break;
    case TRANSPORT_MODE_MULTI:
        set_led_color(0, 0, 32);
        break;
    default:
        set_led_color(32, 32, 32);
        break;
    }
}

void set_transport_mode(transport_mode_t new_mode) 
{
    if(transport_mode != new_mode) {
        transport_mode = new_mode;
        set_transport_mode_led(transport_mode);
    }
}

void get_app_ip(char *out)
{
    strcpy(out, app_ip);
}

static void update_app_ip(otIp6Address host_addr)
{
    char host_addr_string[OT_IP6_ADDRESS_STRING_SIZE];
    otIp6AddressToString(&host_addr, host_addr_string, OT_IP6_ADDRESS_STRING_SIZE);

    if (strcmp(app_ip, host_addr_string))
    {
        strcpy(app_ip, host_addr_string);
        ESP_LOGI(TAG, "Host address updated to %s", app_ip);
    }
}

void app_state_message_handler(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo)
{
    update_app_ip(aMessageInfo->mPeerAddr);

    char message[1024];
    int length = otMessageRead(aMessage, otMessageGetOffset(aMessage), message, sizeof(message) - 1);
    message[length] = '\0';

    ESP_LOGI(TAG, "%s", message);

    transport_mode_t new_transport_mode = message[15] - '0';
    set_transport_mode(new_transport_mode);
}

void init_app_state_message_handler(void)
{
    otSockAddr listenSockAddr;
    otInstance *ot_instance = esp_openthread_get_instance();

    memset(&app_state_socket, 0, sizeof(app_state_socket));
    memset(&listenSockAddr, 0, sizeof(listenSockAddr));

    listenSockAddr.mPort = TRANSPORT_MODE_BROADCAST_PORT;
    set_transport_mode(TRANSPORT_MODE_UDP);

    esp_openthread_lock_acquire(portMAX_DELAY);
    otUdpOpen(ot_instance, &app_state_socket, app_state_message_handler, ot_instance);
    otUdpBind(ot_instance, &app_state_socket, &listenSockAddr, OT_NETIF_THREAD);
    esp_openthread_lock_release();
}
