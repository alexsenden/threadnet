#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "multi_test.h"
#include "network_config.h"
#include "network_state.h"
#include "tcp_test.h"
#include "test_start_receiver.h"
#include "udp_test.h"

#define TAG "Test Runner"

static bool test_active = false;
static int messages_sent;

static void test_worker(void *aContext)
{
    void (*send_test_message)(void) = NULL;
    messages_sent = 0;

    switch(get_transport_mode()){
        case TRANSPORT_MODE_TCP:
            send_test_message = send_tcp_test_message;
            break;
        case TRANSPORT_MODE_UDP:
            send_test_message = send_udp_test_message;
            break;
        case TRANSPORT_MODE_MULTI:
            send_test_message = send_multi_test_message;
            break;
        default:
            ESP_LOGW(TAG, "Failed to run test: undefined transport mode %d", get_transport_mode());
            vTaskDelete(NULL);
            return;
    }

    ESP_LOGI(TAG, "Starting test with transport mode %d.", get_transport_mode());

    test_active = true;
    while(test_active){
        messages_sent++;
        send_test_message();
        vTaskDelay((1000 / TEST_MESSAGES_PER_SECOND) / portTICK_PERIOD_MS);
    }

    ESP_LOGI(TAG, "Test concluded. Send %d messages.", messages_sent);
    vTaskDelete(NULL);
}

static void test_manager_task(void *aContext)
{
    xTaskCreate(test_worker, "test_worker", 10240, xTaskGetCurrentTaskHandle(), 6, NULL);
    vTaskDelay(1000 * TEST_DURATION_SECONDS / portTICK_PERIOD_MS);
    test_active = false;
    vTaskDelete(NULL);
}

void start_test()
{
    if(!test_active){
        xTaskCreate(test_manager_task, "test_manager_task", 10240, xTaskGetCurrentTaskHandle(), 5, NULL);
    }
}
