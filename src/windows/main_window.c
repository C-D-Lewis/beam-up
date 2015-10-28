#include "main_window.h"

static Window *s_window;
static TextLayer *s_digits[NUM_CHARS];

static char s_time_buffer[NUM_CHARS + 1];       // + NULL char
static int s_digit_states_now[NUM_CHARS - 1];   // No colon in this array
static int s_digit_states_prev[NUM_CHARS - 1];

/******************************** Digit logic *********************************/

static void update_digit_values(struct tm *tick_time) {
  strftime(s_time_buffer, sizeof(s_time_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);

  s_digit_states_now[3] = s_time_buffer[4] - '0'; // Convert to int
  s_digit_states_now[2] = s_time_buffer[3] - '0';
  s_digit_states_now[1] = s_time_buffer[1] - '0';
  s_digit_states_now[0] = s_time_buffer[0] - '0';
}

static void show_digit_values() {
  static char s_chars[5][2] = {"1", "2", ":", "3", "4"};
  for(int i = 0; i < NUM_CHARS; i++) {
    if(i != 2) {
      s_chars[i][0] = s_time_buffer[i];
      text_layer_set_text(s_digits[i], DEBUG ? "0" : s_chars[i]);
    } else {
      text_layer_set_text(s_digits[i], ":");
    }
  }
}

static void predict_next_change(struct tm *tick_time) {
  // Update new values
  update_digit_values(tick_time);

  // Hour tens will change
  if(((s_digit_states_now[0] == 0) && (s_digit_states_now[1] == 9) && (s_digit_states_now[2] == 5) && (s_digit_states_now[3] == 9))      //09:59 --> 10:00
  || ((s_digit_states_now[0] == 1) && (s_digit_states_now[1] == 9) && (s_digit_states_now[2] == 5) && (s_digit_states_now[3] == 9))      //19:59 --> 20:00
  || ((s_digit_states_now[0] == 2) && (s_digit_states_now[1] == 3) && (s_digit_states_now[2] == 5) && (s_digit_states_now[3] == 9))) {   //23:59 --> 00:00
    s_digit_states_now[0]++;
  }
 
  // Hour units will change
  if((s_digit_states_now[2] == 5) && (s_digit_states_now[3] == 9)) {
    s_digit_states_now[1]++;
  }
 
  // Minute tens will change
  if(s_digit_states_now[3] == 9) {
    s_digit_states_now[2]++;
  }
     
  // Minute unit always changes
  s_digit_states_now[3]++;
}

/*********************************** Window ***********************************/

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_digits[0] = text_layer_create(GRect(HOURS_TENS_X_OFFSET, Y_OFFSET, DIGIT_SIZE.w, DIGIT_SIZE.h));
  s_digits[1] = text_layer_create(GRect(HOURS_UNITS_X_OFFSET, Y_OFFSET, DIGIT_SIZE.w, DIGIT_SIZE.h));
  s_digits[2] = text_layer_create(GRect(COLON_X_OFFSET, Y_OFFSET, DIGIT_SIZE.w, DIGIT_SIZE.h));
  s_digits[3] = text_layer_create(GRect(MINS_TENS_X_OFFSET, Y_OFFSET, DIGIT_SIZE.w, DIGIT_SIZE.h));
  s_digits[4] = text_layer_create(GRect(MINS_UNITS_X_OFFSET, Y_OFFSET, DIGIT_SIZE.w, DIGIT_SIZE.h));

  for(int i = 0; i < NUM_CHARS; i++) {
    text_layer_set_text_color(s_digits[i], data_get_foreground_color());
    text_layer_set_background_color(s_digits[i], GColorClear);
    text_layer_set_text_alignment(s_digits[i], GTextAlignmentRight);
    text_layer_set_font(s_digits[i], fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_IMAGINE_48)));
    layer_add_child(window_layer, text_layer_get_layer(s_digits[i]));
  }
}

static void window_unload(Window *window) {
  for(int i = 0; i < NUM_CHARS; i++) {
    text_layer_destroy(s_digits[i]);
  }

  window_destroy(s_window);
  s_window = NULL;
}

/************************************ API *************************************/

void main_window_push() {
  // Stop 'all change' on first minute
  for(int i = 0; i < NUM_CHARS - 1; i++) {
    s_digit_states_prev[i] = s_digit_states_now[i];
  }

  if(!s_window) {
    s_window = window_create();
    window_set_background_color(s_window, data_get_background_color());
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
  }
  window_stack_push(s_window, true);

  // Make sure the face is not blank
  time_t temp = time(NULL);  
  struct tm *t = localtime(&temp); 
  update_digit_values(t);
  show_digit_values();
}

void main_window_update_time(struct tm *tick_time) {

}
