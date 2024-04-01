#include "esp_log.h"
#include "esp_openthread.h"
#include "esp_openthread_lock.h"
#include "freertos/FreeRTOS.h"
#include "openthread/udp.h"

#include "network_config.h"
#include "led.h"

#define ADDR_STRING_SIZE 64

#define TAG "Net State"

static transport_mode_t transport_mode = TRANSPORT_MODE_UDP;
static char *network_host_ip = "";

static otUdpReceiver *net_state_receiver;

transport_mode_t get_transport_mode(void)
{
    return transport_mode;
}

void get_network_host_ip(char *out)
{
    strcpy(out, network_host_ip);
}

static void update_network_host_ip(otIp6Address host_addr)
{
    char host_addr_string[ADDR_STRING_SIZE];
    otIp6AddressToString(&host_addr, host_addr_string, ADDR_STRING_SIZE);

    if (strcmp(network_host_ip, host_addr_string))
    {
        strcpy(network_host_ip, host_addr_string);
        ESP_LOGI(TAG, "Host address updated to %s", network_host_ip);
    }
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

static bool net_state_message_handler(void *aContext, const otMessage *aMessage, const otMessageInfo *aMessageInfo)
{
    if (aMessageInfo->mSockPort != TRANSPORT_MODE_BROADCAST_PORT)
    {
        return false;
    }

    update_network_host_ip(aMessageInfo->mPeerAddr);

    char message[1024];
    int length = otMessageRead(aMessage, otMessageGetOffset(aMessage), message, sizeof(message) - 1);
    message[length] = '\0';

    ESP_LOGI(TAG, "%s", message);

    transport_mode = message[15] - '0';
    set_transport_mode_led(transport_mode);

    return true;
}

void init_net_state_message_handler(void)
{
    network_host_ip = malloc(ADDR_STRING_SIZE);

    net_state_receiver = malloc(sizeof(otUdpReceiver));
    net_state_receiver->mHandler = net_state_message_handler;

    otInstance *aInstance = esp_openthread_get_instance();

    // Acquire OT Lock
    esp_openthread_lock_acquire(portMAX_DELAY);

    otUdpAddReceiver(aInstance, net_state_receiver);

    // Release OT Lock
    esp_openthread_lock_release();
}
