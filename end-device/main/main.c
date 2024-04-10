#include <esp_err.h>
#include <esp_vfs_eventfd.h>

#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_netif_types.h"
#include "esp_event.h"

#include "led.h"
#include "app_state.h"
#include "node_state.h"
#include "thread_init.h"

// Entrypoint for end-device firmware
void app_main(void)
{
    esp_vfs_eventfd_config_t eventfd_config = {
        .max_fds = 3,
    };

    // Initialize the eventfd
    ESP_ERROR_CHECK(esp_vfs_eventfd_register(&eventfd_config));

    // Initialize the NVS
    ESP_ERROR_CHECK(nvs_flash_init());

    // Initialize the default netif
    ESP_ERROR_CHECK(esp_netif_init());

    // Create default event loop
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Initialize the LED
    init_led();

    // Initialize the OpenThread network
    start_thread_network();
}
