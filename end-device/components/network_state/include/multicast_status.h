#pragma once

#include <unistd.h>
#include "esp_openthread.h"

void send_status_multicast(otInstance *, void *, uint32_t);
void init_node_status_socket_multicast(otInstance *aInstance);
