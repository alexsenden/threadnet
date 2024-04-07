idf_component_register(SRCS "threadnet_app.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp-tls esp_http_server nvs_flash
                    PRIV_REQUIRES network_state)
