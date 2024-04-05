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

void app_main(void)
{
    esp_vfs_eventfd_config_t eventfd_config = {
        .max_fds = 3,
    };

    ESP_ERROR_CHECK(esp_vfs_eventfd_register(&eventfd_config));
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    init_led();
    start_thread_network();

    init_app_state_message_handler();
    start_node_status_messages();
}
