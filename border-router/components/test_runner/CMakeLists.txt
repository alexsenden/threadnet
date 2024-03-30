set(srcs    "src/test_runner.c"
            "src/udp_test_receiver.c")

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS "include"
                    REQUIRES openthread
                    PRIV_REQUIRES network_state)
