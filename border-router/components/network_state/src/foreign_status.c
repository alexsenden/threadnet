#include "esp_openthread.h"

#include "udp_status.h"

void init_foreign_status_sockets(void) {
    otInstance *ot_instance = esp_openthread_get_instance();

    init_udp_status_socket(ot_instance);
    // Add other sockets here
}