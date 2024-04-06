#include "esp_err.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_netif_types.h"
#include "esp_vfs_eventfd.h"

#include "thread_init.h"
#include "threadnet_app.h"
#include "wifi.h"

void app_main(void)
{
    esp_vfs_eventfd_config_t eventfd_config = {
        .max_fds = 4,
    };

    ESP_ERROR_CHECK(esp_vfs_eventfd_register(&eventfd_config));
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_softap();
    start_thread_network();
    start_threadnet_app();
}
