#pragma once

#include <pebble.h>

#include "../values.h"

#include "../modules/data.h"

#include "../lib/universal_fb/universal_fb.h"

void main_window_push();

void main_window_update_time(struct tm *tick_time);
