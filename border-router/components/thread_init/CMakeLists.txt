idf_component_register(SRCS "src/thread_init.c"
                    INCLUDE_DIRS "include"
                    REQUIRES openthread esp_event esp_netif spiffs vfs nvs_flash
                    PRIV_REQUIRES network_state)
