#pragma once

#include <openthread/ip6.h>
#include <openthread/message.h>

void init_test_listeners(void);

void handle_message(otMessage *, otMessageInfo *);

void start_test(void);

// TODO: remove this in favour of manual test starts
void start_test_loop(void);