/**
 * Beam Up Pebble Watchface
 * Author: Chris Lewis
 */

#include "main.h"

// Global prototypes
TextLayer *s_digits[5];
TextLayer *s_date_layer;
int s_state_now[4];
int s_state_prev[4];
char s_time_buffer[5];

// UI elements
static Window *s_main_window;
static InverterLayer *s_beams[4];
static InverterLayer *s_seconds_layer, *s_inverter_layer;

static bool do_animations;

static void handle_tick(struct tm *t, TimeUnits units_changed) {  
  // Get the time
  int seconds = t->tm_sec;
   
  // Bottom suface
  switch(seconds) {
    // Beam Up!
    case 0:    
      write_time_digits(t);
       
      // Set the time off screen
      show_time_digits(); 
   
      // Animate stuff back into place
      if((s_state_now[3] != s_state_prev[3]) || (DEBUG_MODE)) {     
        animate_layer(text_layer_get_layer(s_digits[4]), GRect(MINS_UNITS_X, -50, 50, 60), GRect(MINS_UNITS_X, 53, 50, 60), 200, 100);
        animate_layer(inverter_layer_get_layer(s_beams[3]), GRect(MINS_UNITS_X + X_OFFSET, 0, BEAM_WIDTH, BEAM_HEIGHT), GRect(MINS_UNITS_X + X_OFFSET, 0, BEAM_WIDTH, 0), 400, 500);
        s_state_prev[3] = s_state_now[3];   // reset the thing
      }
      if((s_state_now[2] != s_state_prev[2]) || (DEBUG_MODE)) {
        animate_layer(text_layer_get_layer(s_digits[3]), GRect(MINS_TENS_X, -50, 50, 60), GRect(MINS_TENS_X, 53, 50, 60), 200, 100);
        animate_layer(inverter_layer_get_layer(s_beams[2]), GRect(MINS_TENS_X + X_OFFSET, 0, BEAM_WIDTH, BEAM_HEIGHT), GRect(MINS_TENS_X + X_OFFSET, 0, BEAM_WIDTH, 0), 400, 500);
        s_state_prev[2] = s_state_now[2];   
      }
      if((s_state_now[1] != s_state_prev[1]) || (DEBUG_MODE)) {     
        animate_layer(text_layer_get_layer(s_digits[1]), GRect(HOURS_UNITS_X, -50, 50, 60), GRect(HOURS_UNITS_X, 53, 50, 60), 200, 100);
        animate_layer(inverter_layer_get_layer(s_beams[1]), GRect(HOURS_UNITS_X + X_OFFSET, 0, BEAM_WIDTH, BEAM_HEIGHT), GRect(HOURS_UNITS_X + X_OFFSET, 0, BEAM_WIDTH, 0), 400, 500);
        s_state_prev[1] = s_state_now[1];   
      }
      if((s_state_now[0] != s_state_prev[0]) || (DEBUG_MODE)) {
        animate_layer(text_layer_get_layer(s_digits[0]), GRect(HOUR_TENS_X, -50, 50, 60), GRect(HOUR_TENS_X, 53, 50, 60), 200, 100);
        animate_layer(inverter_layer_get_layer(s_beams[0]), GRect(HOUR_TENS_X + X_OFFSET, 0, BEAM_WIDTH, BEAM_HEIGHT), GRect(HOUR_TENS_X + X_OFFSET, 0, BEAM_WIDTH, 0), 400, 500);
        s_state_prev[0] = s_state_now[0];   
      }
       
      // Bottom surface down
      animate_layer(inverter_layer_get_layer(s_seconds_layer), GRect(0, 105, 144, 5), GRect(0, 105, 0, 5), 500, 500);
      break;

    // Safetly for bad animation at t=2s
    case 2:
      // Reset TextLayers to show time
      layer_set_frame(text_layer_get_layer(s_digits[0]), GRect(HOUR_TENS_X, 53, 50, 60));
      layer_set_frame(text_layer_get_layer(s_digits[1]), GRect(HOURS_UNITS_X, 53, 50, 60));
      layer_set_frame(text_layer_get_layer(s_digits[3]), GRect(MINS_TENS_X, 53, 50, 60));
      layer_set_frame(text_layer_get_layer(s_digits[4]), GRect(MINS_UNITS_X, 53, 50, 60));

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
      if((s_state_now[3] != s_state_prev[3]) || (DEBUG_MODE)) {
        animate_layer(inverter_layer_get_layer(s_beams[3]), GRect(MINS_UNITS_X + X_OFFSET, 0, BEAM_WIDTH, 0), GRect(MINS_UNITS_X + X_OFFSET, 0, BEAM_WIDTH, BEAM_HEIGHT), 400, 0);
        animate_layer(text_layer_get_layer(s_digits[4]), GRect(MINS_UNITS_X, 53, 50, 60), GRect(MINS_UNITS_X, -50, 50, 60), 200, 700);
      }
       
      // Only change minutes tens if its changed
      if((s_state_now[2] != s_state_prev[2]) || (DEBUG_MODE)) {
        animate_layer(inverter_layer_get_layer(s_beams[2]), GRect(MINS_TENS_X + X_OFFSET, 0, BEAM_WIDTH, 0), GRect(MINS_TENS_X + X_OFFSET, 0, BEAM_WIDTH, BEAM_HEIGHT), 400, 0);
        animate_layer(text_layer_get_layer(s_digits[3]), GRect(MINS_TENS_X, 53, 50, 60), GRect(MINS_TENS_X, -50, 50, 60), 200, 700);
      }
       
      // Only change hours units if its changed
      if((s_state_now[1] != s_state_prev[1]) || (DEBUG_MODE)) {
        animate_layer(inverter_layer_get_layer(s_beams[1]), GRect(HOURS_UNITS_X + X_OFFSET, 0, BEAM_WIDTH, 0), GRect(HOURS_UNITS_X + X_OFFSET, 0, BEAM_WIDTH, BEAM_HEIGHT), 400, 0);
        animate_layer(text_layer_get_layer(s_digits[1]), GRect(HOURS_UNITS_X, 53, 50, 60), GRect(HOURS_UNITS_X, -50, 50, 60), 200, 700);
      }
       
      // Only change hours tens if its changed
      if((s_state_now[0] != s_state_prev[0]) || (DEBUG_MODE)) {
        animate_layer(inverter_layer_get_layer(s_beams[0]), GRect(HOUR_TENS_X + X_OFFSET, 0, BEAM_WIDTH, 0), GRect(HOUR_TENS_X + X_OFFSET, 0, BEAM_WIDTH, BEAM_HEIGHT), 400, 0);
        animate_layer(text_layer_get_layer(s_digits[0]), GRect(HOUR_TENS_X, 53, 50, 60), GRect(HOUR_TENS_X, -50, 50, 60), 200, 700);
      }
      break;      
  }
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  
  // Allocate text layers
  s_digits[0] = gen_text_layer(GRect(HOUR_TENS_X, 53, 50, 60), GColorWhite, GColorClear, true, RESOURCE_ID_FONT_IMAGINE_48, NULL, GTextAlignmentRight);
  layer_add_child(window_layer, text_layer_get_layer(s_digits[0]));

  s_digits[1] = gen_text_layer(GRect(HOURS_UNITS_X, 53, 50, 60), GColorWhite, GColorClear, true, RESOURCE_ID_FONT_IMAGINE_48, NULL, GTextAlignmentRight);
  layer_add_child(window_layer, text_layer_get_layer(s_digits[1]));

  s_digits[2] = gen_text_layer(GRect(68, 53, 50, 60), GColorWhite, GColorClear, true, RESOURCE_ID_FONT_IMAGINE_48, NULL, GTextAlignmentLeft);
  layer_add_child(window_layer, text_layer_get_layer(s_digits[2]));

  s_digits[3] = gen_text_layer(GRect(MINS_TENS_X, 53, 50, 60), GColorWhite, GColorClear, true, RESOURCE_ID_FONT_IMAGINE_48, NULL, GTextAlignmentRight);
  layer_add_child(window_layer, text_layer_get_layer(s_digits[3]));

  s_digits[4] = gen_text_layer(GRect(MINS_UNITS_X, 53, 50, 60), GColorWhite, GColorClear, true, RESOURCE_ID_FONT_IMAGINE_48, NULL, GTextAlignmentRight);
  layer_add_child(window_layer, text_layer_get_layer(s_digits[4]));
  
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
    s_state_prev[i] = s_state_now[i];
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

  // User settings
  s_inverter_layer = inverter_layer_create(GRect(0, 0, 144, 168));
  if(comm_get_setting(PERSIST_KEY_INVERTED)) {
    layer_add_child(window_layer, inverter_layer_get_layer(s_inverter_layer));
  }
  do_animations = comm_get_setting(PERSIST_KEY_ANIM);
}

static void window_unload(Window *window) {  
  // Free text layers
  for(int i = 0; i < 5; i++) {
    if(s_digits[i]) {
      text_layer_destroy(s_digits[i]);
    }
  }
  
  // Free inverter layers
  for(int i = 0; i < 4; i++) {
    if(s_beams[i]) {
      inverter_layer_destroy(s_beams[i]);
    }
  }
  if(s_seconds_layer) {
    inverter_layer_destroy(s_seconds_layer);
  }
  if(s_inverter_layer) {
    inverter_layer_destroy(s_inverter_layer);
  }
  
  // Unsubscribe from events
  tick_timer_service_unsubscribe();
}

static void init(void) {
  comm_setup();

  // Subscribe to events
  tick_timer_service_subscribe(SECOND_UNIT, handle_tick);

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
}

int main(void) {
  init();
  app_event_loop();
  deinit();

  return 0;
}