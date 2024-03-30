set(srcs    "src/multi_test.c"
            "src/tcp_test.c"
            "src/test_runner.c"
            "src/test_start_receiver.c"
            "src/udp_test.c")

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS "include"
                    REQUIRES openthread
                    PRIV_REQUIRES network_state)
