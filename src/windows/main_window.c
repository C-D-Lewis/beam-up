#include "main_window.h"

static Window *s_window;
static TextLayer *s_digits[NUM_CHARS];
static Layer *s_seconds_bar;

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

/********************************* Animation **********************************/

static void animation_stopped_handler(Animation *animation, bool finished, void *context) {
#if defined(PBL_SDK_2)
  property_animation_destroy((PropertyAnimation*)animation);
#endif
}

static void animate_layer(Layer *layer, GRect start, GRect finish, int duration, int delay) {
  PropertyAnimation *prop_anim = property_animation_create_layer_frame(layer, &start, &finish);
  Animation *anim = property_animation_get_animation(prop_anim);
  animation_set_duration(anim, duration);
  animation_set_delay(anim, delay);
  animation_set_handlers(anim, (AnimationHandlers) {
    .stopped = animation_stopped_handler
  }, NULL);
  animation_schedule(anim);
}

static void animate_beams(struct tm *tick_time) {
  GRect bounds = layer_get_bounds(window_get_root_layer(s_window));

  // Get minute progress
  int seconds = tick_time->tm_sec;

  // Hourly vibrate?
  if(data_get_boolean_setting(DataKeyHourlyVibration)) {
    if(tick_time->tm_min == 0 && seconds == 0) {
      // Buzz buzz
      uint32_t segs[] = {200, 300, 200};
      VibePattern pattern = {
        .durations = segs,
        .num_segments = ARRAY_LENGTH(segs)
      };
      vibes_enqueue_custom_pattern(pattern);
    }
  }

  switch(seconds) {
    // Beams up
    case 0: {
      update_digit_values(tick_time);
      show_digit_values();

      // Animate stuff back into place
      Layer *digit_layer;
      if((s_digit_states_now[0] != s_digit_states_prev[0]) || DEBUG) {
        digit_layer = text_layer_get_layer(s_digits[0]);
        animate_layer(digit_layer, layer_get_frame(digit_layer), GRect(HOURS_TENS_X_OFFSET, Y_OFFSET, DIGIT_SIZE.w, DIGIT_SIZE.h), 200, 100);
        s_digit_states_prev[0] = s_digit_states_now[0];
      }
      if((s_digit_states_now[1] != s_digit_states_prev[1]) || DEBUG) {
        digit_layer = text_layer_get_layer(s_digits[1]);
        animate_layer(digit_layer, layer_get_frame(digit_layer), GRect(HOURS_UNITS_X_OFFSET, Y_OFFSET, DIGIT_SIZE.w, DIGIT_SIZE.h), 200, 100);
        s_digit_states_prev[1] = s_digit_states_now[1];   
      }
      if((s_digit_states_now[2] != s_digit_states_prev[2]) || DEBUG) {
        digit_layer = text_layer_get_layer(s_digits[3]);
        animate_layer(digit_layer, layer_get_frame(digit_layer), GRect(MINS_TENS_X_OFFSET, Y_OFFSET, DIGIT_SIZE.w, DIGIT_SIZE.h), 200, 100);
        s_digit_states_prev[2] = s_digit_states_now[2];   
      }
      if((s_digit_states_now[3] != s_digit_states_prev[3]) || DEBUG) {
        digit_layer = text_layer_get_layer(s_digits[4]);
        animate_layer(digit_layer, layer_get_frame(digit_layer), GRect(MINS_UNITS_X_OFFSET, Y_OFFSET, DIGIT_SIZE.w, DIGIT_SIZE.h), 200, 100);
        s_digit_states_prev[3] = s_digit_states_now[3];   
      }
       
      // Bottom surface down
      animate_layer(s_seconds_bar, layer_get_frame(s_seconds_bar), GRect(0, SECONDS_Y_OFFSET, 0, SECONDS_HEIGHT), 500, 500);
    }
      break;

    // 15 seconds bar
    case 15:
      animate_layer(s_seconds_bar, layer_get_frame(s_seconds_bar), GRect(0, SECONDS_Y_OFFSET, bounds.size.w / 4, SECONDS_HEIGHT), 500, 0);
      break;

    // 30 seconds bar
    case 30:
      animate_layer(s_seconds_bar, layer_get_frame(s_seconds_bar), GRect(0, SECONDS_Y_OFFSET, bounds.size.w / 2, SECONDS_HEIGHT), 500, 0);
      break;

    // 45 seconds bar
    case 45:
      animate_layer(s_seconds_bar, layer_get_frame(s_seconds_bar), GRect(0, SECONDS_Y_OFFSET, (3 * bounds.size.w) / 4, SECONDS_HEIGHT), 500, 0);
      break;

    // Complete bar
    case 58:
      animate_layer(s_seconds_bar, layer_get_frame(s_seconds_bar), GRect(0, SECONDS_Y_OFFSET, bounds.size.w, SECONDS_HEIGHT), 500, 1000);
      break;

    // Beams down
    case 59:
      // Predict next changes
      predict_next_change(tick_time);

      if((s_digit_states_now[0] != s_digit_states_prev[0]) || DEBUG) {

      }
       
      if((s_digit_states_now[1] != s_digit_states_prev[1]) || DEBUG) {

      }
       
      if((s_digit_states_now[2] != s_digit_states_prev[2]) || DEBUG) {

      }
       
      if((s_digit_states_now[3] != s_digit_states_prev[3]) || DEBUG) {

      }
      break;    
  }
}

/*********************************** Window ***********************************/

static void seconds_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, data_get_foreground_color());
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
}

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

  s_seconds_bar = layer_create(GRect(0, SECONDS_Y_OFFSET, 0, SECONDS_HEIGHT));
  layer_set_update_proc(s_seconds_bar, seconds_update_proc);
  layer_add_child(window_layer, s_seconds_bar);
}

static void window_unload(Window *window) {
  for(int i = 0; i < NUM_CHARS; i++) {
    text_layer_destroy(s_digits[i]);
  }
  layer_destroy(s_seconds_bar);

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
  struct tm *time_now = localtime(&temp); 
  update_digit_values(time_now);
  show_digit_values();

  // Init seconds bar
  GRect bounds = layer_get_bounds(window_get_root_layer(s_window));
  int seconds = time_now->tm_sec;
  if(seconds >= 15 && seconds < 30) {
    animate_layer(s_seconds_bar, layer_get_frame(s_seconds_bar), GRect(0, SECONDS_Y_OFFSET, bounds.size.w / 4, SECONDS_HEIGHT), 500, 0);
  } else if(seconds >= 30 && seconds < 45) {
    animate_layer(s_seconds_bar, layer_get_frame(s_seconds_bar), GRect(0, SECONDS_Y_OFFSET, bounds.size.w / 2, SECONDS_HEIGHT), 500, 0);
  } else if(seconds >= 45 && seconds < 58) {
    animate_layer(s_seconds_bar, layer_get_frame(s_seconds_bar), GRect(0, SECONDS_Y_OFFSET, (3 * bounds.size.w) / 4, SECONDS_HEIGHT), 500, 0);
  } else if(seconds >= 58) {
    animate_layer(s_seconds_bar, layer_get_frame(s_seconds_bar), GRect(0, SECONDS_Y_OFFSET, bounds.size.w, SECONDS_HEIGHT), 500, 0);
  }
}

void main_window_update_time(struct tm *tick_time) {
  animate_beams(tick_time);
}
