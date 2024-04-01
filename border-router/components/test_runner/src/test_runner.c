#include <stdatomic.h>
#include <string.h>

#include "esp_log.h"
#include "esp_openthread.h"
#include "esp_openthread_lock.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "openthread/ip6.h"
#include "openthread/message.h"
#include "openthread/udp.h"

#include "network_config.h"
#include "udp_test_receiver.h"

#define MAX_DEVICES 64
#define ADDR_STRING_SIZE 64
#define BUFFER_SIZE 32

#define TAG "Test Runner"

typedef struct NODE_RESULTS
{
    char node_ipv6[ADDR_STRING_SIZE];
    atomic_int message_count;
} node_results_t;

static int num_devices = 0;
static node_results_t test_results[MAX_DEVICES];

void handle_message(otMessage *aMessage, otMessageInfo *aMessageInfo)
{
    ESP_LOGI(TAG, "UDP message received");
    // Read the contents of the message - we will want to do this when we are actually
    // extracting network topology data from the nodes

    // char message[1024];
    // int length = otMessageRead(aMessage, otMessageGetOffset(aMessage), message, sizeof(message) - 1);
    // message[length] = '\0';

    char src_addr_string[ADDR_STRING_SIZE];
    otIp6AddressToString(&aMessageInfo->mPeerAddr, src_addr_string, ADDR_STRING_SIZE);

    bool node_found_in_results = false;
    for (int i = 0; i < num_devices && !node_found_in_results; i++)
    {
        if (!strcmp(test_results[i].node_ipv6, src_addr_string))
        {
            test_results[i].message_count++;
        }
    }

    if (!node_found_in_results && num_devices < MAX_DEVICES)
    {
        test_results[num_devices].message_count = 1;
        strcpy(test_results[num_devices].node_ipv6, src_addr_string);
    }
}

static void set_broadcast_message_info(otMessageInfo *message_info)
{
    memset(message_info, 0, sizeof(otMessageInfo));

    message_info->mHopLimit = 0xFF;
    otIp6AddressFromString("ff03::1", &message_info->mSockAddr);
    otIp6AddressFromString("ff03::1", &message_info->mPeerAddr);
    message_info->mSockPort = TEST_START_PORT;
    message_info->mPeerPort = TEST_START_PORT;
}

static void broadcast_test_start(void)
{
    char buf[BUFFER_SIZE];
    sprintf(buf, "STARTING TEST");

    otMessageInfo message_info;
    set_broadcast_message_info(&message_info);

    otInstance *aInstance = esp_openthread_get_instance();

    // Acquire OT Lock
    esp_openthread_lock_acquire(portMAX_DELAY);

    for (int i = 0; i < 3; i++)
    {
        otMessage *test_start_message = otUdpNewMessage(aInstance, NULL);
        otMessageAppend(test_start_message, buf, strlen(buf));

        if (otUdpSendDatagram(aInstance, test_start_message, &message_info) != OT_ERROR_NONE)
        {
            ESP_LOGW(TAG, "Failed to send test start broadcast");
        }
    }

    // Release OT Lock
    esp_openthread_lock_release();
}

static void test_end_worker(void *aContext)
{
    vTaskDelay((TEST_DURATION_SECONDS + TEST_DURATION_BUFFER_SECONDS) * 1000 / portTICK_PERIOD_MS);

    ESP_LOGI(TAG, "Test Results: ------------------------------------");
    for (int i = 0; i < num_devices; i++)
    {
        ESP_LOGI("Test Results", "Host: %s. Messages: %d. Success Rate: %.3f",
                 test_results[i].node_ipv6,
                 test_results[i].message_count,
                 test_results[i].message_count / (float)TEST_NUM_MESSAGES);
    }
    ESP_LOGI(TAG, "End Test Results: --------------------------------");

    vTaskDelete(NULL);
}

void start_test(void)
{
    ESP_LOGI(TAG, "Starting test.");
    num_devices = 0;
    broadcast_test_start();
    xTaskCreate(test_end_worker, "test_end_worker", 10240, xTaskGetCurrentTaskHandle(), 5, NULL);
}

void init_test_listeners(void)
{
    // init_tcp_test_listener(); // To be implemented
    init_udp_test_listener();
    // init_multicast_test_listener(); // To be implemented
}

// TODO: remove this in favour of manual test starts
static void test_loop(void *aContext)
{
    while (1)
    {
        vTaskDelay(2 * (TEST_DURATION_SECONDS + TEST_DURATION_BUFFER_SECONDS) * 1000 / portTICK_PERIOD_MS);
        start_test();
    }
}

// TODO: remove this in favour of manual test starts
void start_test_loop(void)
{
    xTaskCreate(test_loop, "test_loop_worker", 10240, xTaskGetCurrentTaskHandle(), 5, NULL);
}