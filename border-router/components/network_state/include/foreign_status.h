#pragma once

#include "network_config.h"

void init_foreign_status_sockets(void);
void set_ack_packet(ack_packet_t *, threadnet_packet_t *);
