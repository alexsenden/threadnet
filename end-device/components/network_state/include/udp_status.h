#pragma once

#include <unistd.h>
#include "esp_openthread.h"

void send_status_udp(otInstance *, void *, uint32_t);
void init_node_status_socket_udp(otInstance *aInstance);
