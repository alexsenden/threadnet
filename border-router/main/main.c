#include "esp_err.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_netif_types.h"
#include "esp_vfs_eventfd.h"

#include "thread_init.h"
#include "threadnet_app.h"
#include "wifi.h"

// Entrypoint for border-router firmware
void app_main(void)
{
    esp_vfs_eventfd_config_t eventfd_config = {
        .max_fds = 4,
    };

    // Initialize the eventfd
    ESP_ERROR_CHECK(esp_vfs_eventfd_register(&eventfd_config));

    // Initialize the NVS
    ESP_ERROR_CHECK(nvs_flash_init());

    // Initialize the default netif
    ESP_ERROR_CHECK(esp_netif_init());

    // Create default event loop
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Initialize the WiFi softAP
    wifi_init_softap();

    // Initialize the OpenThread network
    start_thread_network();

    // Initialize the Threadnet app
    start_threadnet_app();
}
