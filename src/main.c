/**
 * Beam Up Pebble Watchface
 * Author: Chris Lewis
 */

#include "main.h"

// Global prototypes
TextLayer *g_digits[5];
TextLayer *g_date_layer;
int g_state_now[4];
int g_state_prev[4];
char g_time_buffer[5];
char g_date_buffer[8];
bool g_do_animations;

// UI elements
static Window *s_main_window;
static InverterLayer *s_beams[4];
static InverterLayer *s_seconds_layer, *s_inverter_layer;
static BitmapLayer *s_bt_layer;
static GBitmap *s_bt_bitmap;

static void handle_tick(struct tm *t, TimeUnits units_changed) {  
  // Get the time
  int seconds = t->tm_sec;

  // Hourly vibrate?
  if(comm_get_setting(PERSIST_KEY_HOURLY)) {
    if(t->tm_min == 0 && seconds == 0) {
      // Buzz buzz
      uint32_t segs[] = {200, 300, 200};
      VibePattern pattern = {
        .durations = segs,
        .num_segments = ARRAY_LENGTH(segs)
      };
      vibes_enqueue_custom_pattern(pattern);
    }
  }
   
  // Bottom suface
  switch(seconds) {
    // Beam Up!
    case 0:    
      write_time_digits(t);
       
      // Set the time off screen
      show_time_digits(); 
   
      // Animate stuff back into place
      if((g_state_now[3] != g_state_prev[3]) || (DEBUG_MODE)) {     
        animate_layer(text_layer_get_layer(g_digits[4]), GRect(MINS_UNITS_X, -50, 50, 60), GRect(MINS_UNITS_X, 53, 50, 60), 200, 100);
        animate_layer(inverter_layer_get_layer(s_beams[3]), GRect(MINS_UNITS_X + X_OFFSET, 0, BEAM_WIDTH, BEAM_HEIGHT), GRect(MINS_UNITS_X + X_OFFSET, 0, BEAM_WIDTH, 0), 400, 500);
        g_state_prev[3] = g_state_now[3];   // reset the thing
      }
      if((g_state_now[2] != g_state_prev[2]) || (DEBUG_MODE)) {
        animate_layer(text_layer_get_layer(g_digits[3]), GRect(MINS_TENS_X, -50, 50, 60), GRect(MINS_TENS_X, 53, 50, 60), 200, 100);
        animate_layer(inverter_layer_get_layer(s_beams[2]), GRect(MINS_TENS_X + X_OFFSET, 0, BEAM_WIDTH, BEAM_HEIGHT), GRect(MINS_TENS_X + X_OFFSET, 0, BEAM_WIDTH, 0), 400, 500);
        g_state_prev[2] = g_state_now[2];   
      }
      if((g_state_now[1] != g_state_prev[1]) || (DEBUG_MODE)) {     
        animate_layer(text_layer_get_layer(g_digits[1]), GRect(HOURS_UNITS_X, -50, 50, 60), GRect(HOURS_UNITS_X, 53, 50, 60), 200, 100);
        animate_layer(inverter_layer_get_layer(s_beams[1]), GRect(HOURS_UNITS_X + X_OFFSET, 0, BEAM_WIDTH, BEAM_HEIGHT), GRect(HOURS_UNITS_X + X_OFFSET, 0, BEAM_WIDTH, 0), 400, 500);
        g_state_prev[1] = g_state_now[1];   
      }
      if((g_state_now[0] != g_state_prev[0]) || (DEBUG_MODE)) {
        animate_layer(text_layer_get_layer(g_digits[0]), GRect(HOUR_TENS_X, -50, 50, 60), GRect(HOUR_TENS_X, 53, 50, 60), 200, 100);
        animate_layer(inverter_layer_get_layer(s_beams[0]), GRect(HOUR_TENS_X + X_OFFSET, 0, BEAM_WIDTH, BEAM_HEIGHT), GRect(HOUR_TENS_X + X_OFFSET, 0, BEAM_WIDTH, 0), 400, 500);
        g_state_prev[0] = g_state_now[0];   
      }
       
      // Bottom surface down
      animate_layer(inverter_layer_get_layer(s_seconds_layer), GRect(0, 105, 144, 5), GRect(0, 105, 0, 5), 500, 500);
      break;

    // Safetly for bad animation at t=2s
    case 2:
      // Reset TextLayers to show time
      layer_set_frame(text_layer_get_layer(g_digits[0]), GRect(HOUR_TENS_X, 53, 50, 60));
      layer_set_frame(text_layer_get_layer(g_digits[1]), GRect(HOURS_UNITS_X, 53, 50, 60));
      layer_set_frame(text_layer_get_layer(g_digits[3]), GRect(MINS_TENS_X, 53, 50, 60));
      layer_set_frame(text_layer_get_layer(g_digits[4]), GRect(MINS_UNITS_X, 53, 50, 60));

      // Reset InverterLayers
      layer_set_frame(inverter_layer_get_layer(s_beams[0]), GRect(0, 0, 0, 0));
      layer_set_frame(inverter_layer_get_layer(s_beams[1]), GRect(0, 0, 0, 0));
      layer_set_frame(inverter_layer_get_layer(s_beams[2]), GRect(0, 0, 0, 0));
      layer_set_frame(inverter_layer_get_layer(s_beams[3]), GRect(0, 0, 0, 0));

      // Get the time
      show_time_digits(t);
      break;

    // 15 seconds bar
    case 15:
      animate_layer(inverter_layer_get_layer(s_seconds_layer), GRect(0, 105, 0, 5), GRect(0, 105, 36, 5), 500, 0);
      break;

    // 30 seconds bar
    case 30:
      animate_layer(inverter_layer_get_layer(s_seconds_layer), GRect(0, 105, 36, 5), GRect(0, 105, 72, 5), 500, 0);
      break;

    // 45 seconds bar
    case 45:
      animate_layer(inverter_layer_get_layer(s_seconds_layer), GRect(0, 105, 72, 5), GRect(0, 105, 108, 5), 500, 0);
      break;

    // Complete bar
    case 58:
      animate_layer(inverter_layer_get_layer(s_seconds_layer), GRect(0, 105, 108, 5), GRect(0, 105, 144, 5), 500, 1000);
      break;

    // Beam down
    case 59:
      // Predict next changes
      predict_next_change(t); // CALLS write_time_digits()
       
      // Only change minutes units if its changed
      if((g_state_now[3] != g_state_prev[3]) || (DEBUG_MODE)) {
        animate_layer(inverter_layer_get_layer(s_beams[3]), GRect(MINS_UNITS_X + X_OFFSET, 0, BEAM_WIDTH, 0), GRect(MINS_UNITS_X + X_OFFSET, 0, BEAM_WIDTH, BEAM_HEIGHT), 400, 0);
        animate_layer(text_layer_get_layer(g_digits[4]), GRect(MINS_UNITS_X, 53, 50, 60), GRect(MINS_UNITS_X, -50, 50, 60), 200, 700);
      }
       
      // Only change minutes tens if its changed
      if((g_state_now[2] != g_state_prev[2]) || (DEBUG_MODE)) {
        animate_layer(inverter_layer_get_layer(s_beams[2]), GRect(MINS_TENS_X + X_OFFSET, 0, BEAM_WIDTH, 0), GRect(MINS_TENS_X + X_OFFSET, 0, BEAM_WIDTH, BEAM_HEIGHT), 400, 0);
        animate_layer(text_layer_get_layer(g_digits[3]), GRect(MINS_TENS_X, 53, 50, 60), GRect(MINS_TENS_X, -50, 50, 60), 200, 700);
      }
       
      // Only change hours units if its changed
      if((g_state_now[1] != g_state_prev[1]) || (DEBUG_MODE)) {
        animate_layer(inverter_layer_get_layer(s_beams[1]), GRect(HOURS_UNITS_X + X_OFFSET, 0, BEAM_WIDTH, 0), GRect(HOURS_UNITS_X + X_OFFSET, 0, BEAM_WIDTH, BEAM_HEIGHT), 400, 0);
        animate_layer(text_layer_get_layer(g_digits[1]), GRect(HOURS_UNITS_X, 53, 50, 60), GRect(HOURS_UNITS_X, -50, 50, 60), 200, 700);
      }
       
      // Only change hours tens if its changed
      if((g_state_now[0] != g_state_prev[0]) || (DEBUG_MODE)) {
        animate_layer(inverter_layer_get_layer(s_beams[0]), GRect(HOUR_TENS_X + X_OFFSET, 0, BEAM_WIDTH, 0), GRect(HOUR_TENS_X + X_OFFSET, 0, BEAM_WIDTH, BEAM_HEIGHT), 400, 0);
        animate_layer(text_layer_get_layer(g_digits[0]), GRect(HOUR_TENS_X, 53, 50, 60), GRect(HOUR_TENS_X, -50, 50, 60), 200, 700);
      }
      break;      
  }
}

static void bt_handler(bool connected) {
  if(connected) {
    layer_set_hidden(bitmap_layer_get_layer(s_bt_layer), true);
  } else {
    vibes_short_pulse();
    layer_set_hidden(bitmap_layer_get_layer(s_bt_layer), false);
  }
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  
  // Allocate text layers
  g_digits[0] = gen_text_layer(GRect(HOUR_TENS_X, 53, 50, 60), GColorWhite, GColorClear, true, RESOURCE_ID_FONT_IMAGINE_48, NULL, GTextAlignmentRight);
  layer_add_child(window_layer, text_layer_get_layer(g_digits[0]));

  g_digits[1] = gen_text_layer(GRect(HOURS_UNITS_X, 53, 50, 60), GColorWhite, GColorClear, true, RESOURCE_ID_FONT_IMAGINE_48, NULL, GTextAlignmentRight);
  layer_add_child(window_layer, text_layer_get_layer(g_digits[1]));

  g_digits[2] = gen_text_layer(GRect(68, 53, 50, 60), GColorWhite, GColorClear, true, RESOURCE_ID_FONT_IMAGINE_48, NULL, GTextAlignmentLeft);
  layer_add_child(window_layer, text_layer_get_layer(g_digits[2]));

  g_digits[3] = gen_text_layer(GRect(MINS_TENS_X, 53, 50, 60), GColorWhite, GColorClear, true, RESOURCE_ID_FONT_IMAGINE_48, NULL, GTextAlignmentRight);
  layer_add_child(window_layer, text_layer_get_layer(g_digits[3]));

  g_digits[4] = gen_text_layer(GRect(MINS_UNITS_X, 53, 50, 60), GColorWhite, GColorClear, true, RESOURCE_ID_FONT_IMAGINE_48, NULL, GTextAlignmentRight);
  layer_add_child(window_layer, text_layer_get_layer(g_digits[4]));

  // Allocate inverter layers
  for(int i = 0; i < 4; i++) {
    s_beams[i] = inverter_layer_create(GRect(0, 0, BEAM_WIDTH, 0));
    layer_add_child(window_layer, inverter_layer_get_layer(s_beams[i]));  
  }
  s_seconds_layer = inverter_layer_create(GRect(0, 0, 144, 0));
  layer_add_child(window_layer, inverter_layer_get_layer(s_seconds_layer));

  // Make sure the face is not blank
  time_t temp = time(NULL);  
  struct tm *t = localtime(&temp);  
  write_time_digits(t);
  
  // Stop 'all change' on firsinute
  for(int i = 0; i < 4; i++) {
    g_state_prev[i] = g_state_now[i];
  }

  // User settings
  g_do_animations = comm_get_setting(PERSIST_KEY_ANIM);
  g_date_layer = gen_text_layer(GRect(45, 105, 100, 30), GColorWhite, GColorClear, true, RESOURCE_ID_FONT_IMAGINE_24, NULL, GTextAlignmentRight);
  if(comm_get_setting(PERSIST_KEY_DATE)) {
    layer_add_child(window_layer, text_layer_get_layer(g_date_layer));
  }
  s_bt_layer = bitmap_layer_create(GRect(59, 140, 27, 26));
  s_bt_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BT);
  bitmap_layer_set_bitmap(s_bt_layer, s_bt_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_bt_layer));
  layer_set_hidden(bitmap_layer_get_layer(s_bt_layer), true);
  if(comm_get_setting(PERSIST_KEY_BT)) {
    if(!bluetooth_connection_service_peek()) {
      layer_set_hidden(bitmap_layer_get_layer(s_bt_layer), false);
    }
  }
  s_inverter_layer = inverter_layer_create(GRect(0, 0, 144, 168));
  if(comm_get_setting(PERSIST_KEY_INVERTED)) {
    layer_add_child(window_layer, inverter_layer_get_layer(s_inverter_layer));
  }

  // Set time digits now  
  show_time_digits();

  // Init seconds bar
  int seconds = t->tm_sec;
  if(seconds >= 15 && seconds < 30) {
    animate_layer(inverter_layer_get_layer(s_seconds_layer), GRect(0, 105, 0, 5), GRect(0, 105, 36, 5), 500, 0);
  } else if(seconds >= 30 && seconds < 45) {
    animate_layer(inverter_layer_get_layer(s_seconds_layer), GRect(0, 105, 0, 5), GRect(0, 105, 72, 5), 500, 0);
  } else if(seconds >= 45 && seconds < 58) {
    animate_layer(inverter_layer_get_layer(s_seconds_layer), GRect(0, 105, 0, 5), GRect(0, 105, 108, 5), 500, 0);
  } else if(seconds >= 58) {
    animate_layer(inverter_layer_get_layer(s_seconds_layer), GRect(0, 105, 0, 5), GRect(0, 105, 144, 5), 500, 1000);
  }
}

static void window_unload(Window *window) {  
  // Free text layers
  for(int i = 0; i < 5; i++) {
    text_layer_destroy(g_digits[i]);
  }
  text_layer_destroy(g_date_layer);

  bitmap_layer_destroy(s_bt_layer);
  gbitmap_destroy(s_bt_bitmap);
  
  // Free inverter layers
  for(int i = 0; i < 4; i++) {
    inverter_layer_destroy(s_beams[i]);
  }
  inverter_layer_destroy(s_seconds_layer);
  inverter_layer_destroy(s_inverter_layer);
}

static void init(void) {
  // Prepare to receive app config
  comm_setup();

  // Localize
  setlocale(LC_ALL, "");

  // Subscribe to events
  tick_timer_service_subscribe(SECOND_UNIT, handle_tick);

  if(comm_get_setting(PERSIST_KEY_BT)) {
    bluetooth_connection_service_subscribe(bt_handler);
  }

  // Create main window
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload
  });
  window_stack_push(s_main_window, true);
}

static void deinit(void) {
  window_destroy(s_main_window);

  // Unsubscribe from events
  tick_timer_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
}

int main(void) {
  init();
  app_event_loop();
  deinit();

  return 0;
}