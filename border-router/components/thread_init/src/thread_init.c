#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_netif_types.h"
#include "esp_openthread.h"
#include "esp_openthread_cli.h"
#include "esp_openthread_lock.h"
#include "esp_openthread_netif_glue.h"
#include "esp_openthread_types.h"
#include "esp_ot_config.h"
#include "esp_spiffs.h"
#include "esp_rcp_update.h"
#include "esp_vfs_eventfd.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hal/uart_types.h"
#include "nvs_flash.h"
#include "openthread/cli.h"
#include "openthread/instance.h"
#include "openthread/logging.h"
#include "openthread/tasklet.h"
#include "openthread/udp.h"
#include "openthread/message.h"

#include "app_state.h"
#include "foreign_status.h"
#include "node_state.h"
#include "thread_init.h"

#define TAG "thread_worker"

// Initialize the SPIFFS filesystem required for the ESP-IDF
static esp_err_t init_spiffs(void)
{
    esp_vfs_spiffs_conf_t rcp_fw_conf = {
        .base_path = "/rcp_fw", .partition_label = "rcp_fw", .max_files = 10, .format_if_mount_failed = false};
    ESP_ERROR_CHECK(esp_vfs_spiffs_register(&rcp_fw_conf));

    return ESP_OK;
}

// Initialize the OpenThread netif
static esp_netif_t *init_openthread_netif(const esp_openthread_platform_config_t *config)
{
    // Use the default OpenThread netif configuration
    esp_netif_config_t cfg = ESP_NETIF_DEFAULT_OPENTHREAD();

    // Create the OpenThread netif
    esp_netif_t *netif = esp_netif_new(&cfg);

    // Check for errors
    assert(netif != NULL);
    ESP_ERROR_CHECK(esp_netif_attach(netif, esp_openthread_netif_glue_init(config)));

    return netif;
}

// Initialize the task that handles the OpenThread stack
static void ot_task_worker(void *aContext)
{
    esp_openthread_platform_config_t config = {
        .radio_config = ESP_OPENTHREAD_DEFAULT_RADIO_CONFIG(),
        .host_config = ESP_OPENTHREAD_DEFAULT_HOST_CONFIG(),
        .port_config = ESP_OPENTHREAD_DEFAULT_PORT_CONFIG(),
    };

    // Initialize the OpenThread stack
    ESP_ERROR_CHECK(esp_openthread_init(&config));

    // Acquire the OpenThread lock
    esp_openthread_lock_acquire(portMAX_DELAY);

    // Initialize the esp_netif bindings
    esp_netif_t *openthread_netif;
    openthread_netif = init_openthread_netif(&config);
    esp_netif_set_default_netif(openthread_netif);

    // Initialize the foreign status sockets
    init_foreign_status_sockets();

    // Initialize the node status messages
    start_node_status_messages();

    // Initialize the network state broadcasts
    start_net_state_broadcasts();

    ESP_ERROR_CHECK(esp_openthread_auto_start(NULL));

    // Release the OpenThread lock
    esp_openthread_lock_release();

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
    // Initialize the SPIFFS filesystem
    ESP_ERROR_CHECK(init_spiffs());

    // Initialize the RCP update service
    esp_rcp_update_config_t rcp_update_config = ESP_OPENTHREAD_RCP_UPDATE_CONFIG();
    ESP_ERROR_CHECK(esp_rcp_update_init(&rcp_update_config));

    // Launch the OpenThread task
    xTaskCreate(ot_task_worker, "ot_main_worker", 10240, xTaskGetCurrentTaskHandle(), 5, NULL);
}
