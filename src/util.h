#pragma once

#include <pebble.h>

#include "globals.h"

void util_write_time_digits(struct tm *t);
void util_show_time_digits();
void util_predict_next_change(struct tm *t);
TextLayer* util_gen_text_layer(GRect location, GColor colour, GColor background, bool custom_font, int custom_res_id, const char *res_id, GTextAlignment alignment);
void util_animate_layer(Layer *layer, GRect start, GRect finish, int duration, int delay);
void util_interpret_message_result(AppMessageResult app_message_error);