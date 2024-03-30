#pragma once

#include "network_config.h"
#include <openthread/ip6.h>

transport_mode_t get_transport_mode(void);

void get_network_host_ip(char*);

void init_net_state_message_handler(void);
