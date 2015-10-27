#pragma once

#include <pebble.h>

#define NUM_CHARS            5
#define Y_OFFSET             53
#define BEAM_SIZE            GSize(30, 101)
#define DIGIT_SIZE           GSize(50, 60)
#define HOURS_TENS_X_OFFSET   PBL_IF_ROUND_ELSE(5, -13)
#define HOURS_UNITS_X_OFFSET PBL_IF_ROUND_ELSE(39, 21)
#define COLON_X_OFFSET       HOURS_UNITS_X_OFFSET + PBL_IF_ROUND_ELSE(47, 65)
#define MINS_TENS_X_OFFSET   PBL_IF_ROUND_ELSE(81, 63)
#define MINS_UNITS_X_OFFSET  PBL_IF_ROUND_ELSE(115, 97)