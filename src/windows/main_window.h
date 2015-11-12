#pragma once

#include <pebble.h>

#include "../config.h"

#include "../modules/data.h"
#include "../modules/compat.h"

#include "../lib/universal_fb/universal_fb.h"
#include "../lib/owm_weather/owm_weather.h"

void main_window_push();

void main_window_update_time(struct tm *tick_time);
