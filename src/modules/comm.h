#pragma once

#include <pebble.h>

#include "data.h"

void comm_init(uint32_t inbox, uint32_t outbox, bool maximum);

void comm_deinit();
