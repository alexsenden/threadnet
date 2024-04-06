set(srcs    "src/app_state.c"
            "src/multicast_status"
            "src/node_state.c"
            "src/udp_status.c")

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS "include"
                    REQUIRES openthread led)
