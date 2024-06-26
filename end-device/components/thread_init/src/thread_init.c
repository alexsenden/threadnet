#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "esp_err.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_netif_types.h"
#include "esp_openthread.h"
#include "esp_openthread_lock.h"
#include "esp_openthread_netif_glue.h"
#include "esp_openthread_types.h"
#include "esp_ot_config.h"
#include "esp_vfs_eventfd.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "openthread/cli.h"
#include "openthread/instance.h"
#include "openthread/logging.h"
#include "openthread/tasklet.h"
#include "openthread/udp.h"

#include "app_state.h"
#include "node_state.h"

#define TAG "thread_worker"

// Initialize the SPIFFS filesystem required for the ESP-IDF
static esp_netif_t *init_openthread_netif(const esp_openthread_platform_config_t *config)
{
    esp_netif_config_t cfg = ESP_NETIF_DEFAULT_OPENTHREAD();
    esp_netif_t *netif = esp_netif_new(&cfg);
    assert(netif != NULL);
    ESP_ERROR_CHECK(esp_netif_attach(netif, esp_openthread_netif_glue_init(config)));

    return netif;
}

// Start the OpenThread network task
static void ot_task_worker(void *aContext)
{
    esp_openthread_platform_config_t config = {
        .radio_config = ESP_OPENTHREAD_DEFAULT_RADIO_CONFIG(),
        .host_config = ESP_OPENTHREAD_DEFAULT_HOST_CONFIG(),
        .port_config = ESP_OPENTHREAD_DEFAULT_PORT_CONFIG(),
    };

    // Initialize the OpenThread stack
    ESP_ERROR_CHECK(esp_openthread_init(&config));

    esp_netif_t *openthread_netif;

    // Initialize the esp_netif bindings with default OpenThread configuration
    openthread_netif = init_openthread_netif(&config);
    esp_netif_set_default_netif(openthread_netif);

    // Initialize the OpenThread lock
    esp_openthread_lock_init();

    // Start node status messages
    start_node_status_messages();

    // Initialize the app state message receiver
    init_app_state_message_handler();

    ESP_ERROR_CHECK(esp_openthread_auto_start(NULL));

    // Launch the OpenThread main loop (blocking call)
    esp_openthread_launch_mainloop();

    // Clean up
    esp_openthread_netif_glue_deinit();
    esp_netif_destroy(openthread_netif);
    esp_vfs_eventfd_unregister();
    vTaskDelete(NULL);
}

// Start the network thread
void start_thread_network(void)
{
    // Create the task that handles the OpenThread stack
    xTaskCreate(ot_task_worker, "ot_main_worker", 10240, xTaskGetCurrentTaskHandle(), 5, NULL);
    ESP_LOGI(TAG, "Thread network started");
}
