#pragma once

#include "network_config.h"

void set_transport_mode(transport_mode_t);

transport_mode_t get_transport_mode();

void start_net_state_broadcasts();
