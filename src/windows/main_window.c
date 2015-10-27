#include "main_window.h"

static Window *s_window;
static TextLayer *s_digits[NUM_DIGITS];

static char s_time_buffer[NUM_DIGITS + 1];
static int s_digit_states_now[NUM_DIGITS];
static int s_digit_states_prev[NUM_DIGITS];

/******************************** Digit logic *********************************/

static void update_digit_values(struct tm *tick_time) {
  strftime(s_time_buffer, sizeof(s_time_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);

  s_digit_states_now[4] = s_time_buffer[4] - '0';
  s_digit_states_now[3] = s_time_buffer[3] - '0';
  s_digit_states_now[2] = 0;  // Unused
  s_digit_states_now[1] = s_time_buffer[1] - '0';
  s_digit_states_now[0] = s_time_buffer[0] - '0';
}

static void show_digit_values() {
  for(int i = 0; i < NUM_DIGITS; i++) {
    text_layer_set_text(s_digits[i], &s_time_buffer[i]);
  }
}

/*********************************** Window ***********************************/

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_digits[0] = text_layer_create(GRect(HOURS_TENS_X_OFFSET, Y_OFFSET, DIGIT_SIZE.w, DIGIT_SIZE.h));
  s_digits[0] = text_layer_create(GRect(HOURS_UNITS_X_OFFSET, Y_OFFSET, DIGIT_SIZE.w, DIGIT_SIZE.h));
  s_digits[0] = text_layer_create(GRect(COLON_X_OFFSET, Y_OFFSET, DIGIT_SIZE.w, DIGIT_SIZE.h));
  s_digits[0] = text_layer_create(GRect(MINS_TENS_X_OFFSET, Y_OFFSET, DIGIT_SIZE.w, DIGIT_SIZE.h));
  s_digits[0] = text_layer_create(GRect(MINS_TENS_X_OFFSET, Y_OFFSET, DIGIT_SIZE.w, DIGIT_SIZE.h));

  for(int i = 0; i < NUM_DIGITS; i++) {
    text_layer_set_text_color(s_digits[i], data_get_foreground_color());
    text_layer_set_background_color(s_digits[i], GColorClear);
    text_layer_set_font(s_digits[i], fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_IMAGINE_48)));
    layer_add_child(window_layer, text_layer_get_layer(s_digits[i]));
  }
}

static void window_unload(Window *window) {
  for(int i = 0; i < NUM_DIGITS; i++) {
    text_layer_destroy(s_digits[0]);
  }

  window_destroy(s_window);
  s_window = NULL;
}

/************************************ API *************************************/

void main_window_push() {
  if(!s_window) {
    s_window = window_create();
    window_set_background_color(s_window, data_get_background_color());
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
  }
  window_stack_push(s_window, true);
}

void main_window_update_time(struct tm *tick_time) {

}
