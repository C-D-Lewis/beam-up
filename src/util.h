#pragma once

#include "main.h"

/**
 * See util.c for function documentation
 */

void write_time_digits(struct tm *t);

void show_time_digits();

void predict_next_change(struct tm *t);

TextLayer* gen_text_layer(GRect location, GColor colour, GColor background, bool custom_font, int custom_res_id, const char *res_id, GTextAlignment alignment);

void animate_layer(Layer *layer, GRect start, GRect finish, int duration, int delay);

void interpret_message_result(AppMessageResult app_message_error);