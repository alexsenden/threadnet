idf_component_register(SRCS "src/thread_init.c"
                    INCLUDE_DIRS "include"
                    REQUIRES openthread esp_event vfs nvs_flash)
