set(srcs    "src/app_state.c"
            "src/foreign_status.c"
            "src/multicast_status.c"
            "src/node_state.c"
            "src/udp_status.c")

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS "include"
                    REQUIRES openthread
                    PRIV_REQUIRES threadnet_app)
