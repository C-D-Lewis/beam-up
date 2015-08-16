/**
 * Beam Up Pebble Watchface
 * Author: Chris Lewis
 */

#include "globals.h"
#include "comm.h"

#include "shim/InverterLayerCompat.h"

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
#ifdef PBL_PLATFORM_APLITE
static InverterLayer *s_beams[4];
static InverterLayer *s_seconds_layer, *s_battery_layer;
#elif PBL_PLATFORM_BASALT
static InverterLayerCompat *s_beams[4];
static InverterLayerCompat *s_seconds_layer, *s_battery_layer;
#endif
static BitmapLayer *s_bt_layer;
static GBitmap *s_bt_bitmap;

// Data
static bool s_tapped;
static GColor bg_color, fg_color;

#ifdef PBL_PLATFORM_APLITE
static InverterLayer* create_inv_layer(GRect bounds) {
  return inverter_layer_create(bounds);
}
static Layer* get_inv_layer(InverterLayer *this) {
  return inverter_layer_get_layer(this);
}
static void destroy_inv_layer(InverterLayer *this) {
  inverter_layer_destroy(this);
}
#elif PBL_PLATFORM_BASALT
static InverterLayerCompat* create_inv_layer(GRect bounds) {
  return inverter_layer_compat_create(bounds);
}
static Layer* get_inv_layer(InverterLayerCompat *this) {
  return inverter_layer_compat_get_layer(this);
}
static void destroy_inv_layer(InverterLayerCompat *this) {
  inverter_layer_compat_destroy(this);
}
#endif

static void handle_tick(struct tm *t, TimeUnits units_changed) {
  // Get the time
  int seconds = t->tm_sec;

  // Hourly vibrate?
  if(comm_get_setting(PERSIST_KEY_HOURLY)) {
    if(t->tm_min == 0 && seconds == 0) {
      uint32_t segs[] = {200, 300, 200};
      VibePattern pattern = {
        .durations = segs,
        .num_segments = ARRAY_LENGTH(segs)
      };

      switch (comm_get_setting_value(PERSIST_KEY_H_VIBE)) {
      case 0:
        // Buzz buzz
        vibes_enqueue_custom_pattern(pattern);
        break;
      case 1:
        vibes_short_pulse();
        break;
      case 2:
        vibes_double_pulse();
        break;
      case 3:
        vibes_long_pulse();
        break;
      }
    }
  }

  // Bottom suface
  switch(seconds) {
    // Beam Up!
    case 0:
      util_write_time_digits(t);

      // Set the time off screen
      util_show_time_digits();

      // Animate stuff back into place
      if((g_state_now[3] != g_state_prev[3]) || (DEBUG_MODE)) {
        util_animate_layer(text_layer_get_layer(g_digits[4]), GRect(MINS_UNITS_X, -50, 50, 60), GRect(MINS_UNITS_X, 53, 50, 60), 200, 100);
        util_animate_layer(get_inv_layer(s_beams[3]), GRect(MINS_UNITS_X + X_OFFSET, 0, BEAM_WIDTH, BEAM_HEIGHT), GRect(MINS_UNITS_X + X_OFFSET, 0, BEAM_WIDTH, 0), 400, 500);
        g_state_prev[3] = g_state_now[3];   // reset the thing
      }
      if((g_state_now[2] != g_state_prev[2]) || (DEBUG_MODE)) {
        util_animate_layer(text_layer_get_layer(g_digits[3]), GRect(MINS_TENS_X, -50, 50, 60), GRect(MINS_TENS_X, 53, 50, 60), 200, 100);
        util_animate_layer(get_inv_layer(s_beams[2]), GRect(MINS_TENS_X + X_OFFSET, 0, BEAM_WIDTH, BEAM_HEIGHT), GRect(MINS_TENS_X + X_OFFSET, 0, BEAM_WIDTH, 0), 400, 500);
        g_state_prev[2] = g_state_now[2];
      }
      if((g_state_now[1] != g_state_prev[1]) || (DEBUG_MODE)) {
        util_animate_layer(text_layer_get_layer(g_digits[1]), GRect(HOURS_UNITS_X, -50, 50, 60), GRect(HOURS_UNITS_X, 53, 50, 60), 200, 100);
        util_animate_layer(get_inv_layer(s_beams[1]), GRect(HOURS_UNITS_X + X_OFFSET, 0, BEAM_WIDTH, BEAM_HEIGHT), GRect(HOURS_UNITS_X + X_OFFSET, 0, BEAM_WIDTH, 0), 400, 500);
        g_state_prev[1] = g_state_now[1];
      }
      if((g_state_now[0] != g_state_prev[0]) || (DEBUG_MODE)) {
        util_animate_layer(text_layer_get_layer(g_digits[0]), GRect(HOUR_TENS_X, -50, 50, 60), GRect(HOUR_TENS_X, 53, 50, 60), 200, 100);
        util_animate_layer(get_inv_layer(s_beams[0]), GRect(HOUR_TENS_X + X_OFFSET, 0, BEAM_WIDTH, BEAM_HEIGHT), GRect(HOUR_TENS_X + X_OFFSET, 0, BEAM_WIDTH, 0), 400, 500);
        g_state_prev[0] = g_state_now[0];
      }

      // Bottom surface down
      util_animate_layer(get_inv_layer(s_seconds_layer), GRect(0, 105, 144, 5), GRect(0, 105, 0, 5), 500, 500);
      break;

    // Safetly for bad animation at t=2s
    case 2:
      // Reset TextLayers to show time
      layer_set_frame(text_layer_get_layer(g_digits[0]), GRect(HOUR_TENS_X, 53, 50, 60));
      layer_set_frame(text_layer_get_layer(g_digits[1]), GRect(HOURS_UNITS_X, 53, 50, 60));
      layer_set_frame(text_layer_get_layer(g_digits[3]), GRect(MINS_TENS_X, 53, 50, 60));
      layer_set_frame(text_layer_get_layer(g_digits[4]), GRect(MINS_UNITS_X, 53, 50, 60));

      // Reset InverterLayers
      layer_set_frame(get_inv_layer(s_beams[0]), GRect(0, 0, 0, 0));
      layer_set_frame(get_inv_layer(s_beams[1]), GRect(0, 0, 0, 0));
      layer_set_frame(get_inv_layer(s_beams[2]), GRect(0, 0, 0, 0));
      layer_set_frame(get_inv_layer(s_beams[3]), GRect(0, 0, 0, 0));

      // Get the time
      util_show_time_digits(t);
      break;

    // 15 seconds bar
    case 15:
      util_animate_layer(get_inv_layer(s_seconds_layer), GRect(0, 105, 0, 5), GRect(0, 105, 36, 5), 500, 0);
      break;

    // 30 seconds bar
    case 30:
      util_animate_layer(get_inv_layer(s_seconds_layer), GRect(0, 105, 36, 5), GRect(0, 105, 72, 5), 500, 0);
      break;

    // 45 seconds bar
    case 45:
      util_animate_layer(get_inv_layer(s_seconds_layer), GRect(0, 105, 72, 5), GRect(0, 105, 108, 5), 500, 0);
      break;

    // Complete bar
    case 58:
      util_animate_layer(get_inv_layer(s_seconds_layer), GRect(0, 105, 108, 5), GRect(0, 105, 144, 5), 500, 1000);
      break;

    // Beam down
    case 59:
      // Predict next changes
      util_predict_next_change(t); // CALLS util_write_time_digits()

      // Only change minutes units if its changed
      if((g_state_now[3] != g_state_prev[3]) || (DEBUG_MODE)) {
        util_animate_layer(get_inv_layer(s_beams[3]), GRect(MINS_UNITS_X + X_OFFSET, 0, BEAM_WIDTH, 0), GRect(MINS_UNITS_X + X_OFFSET, 0, BEAM_WIDTH, BEAM_HEIGHT), 400, 0);
        util_animate_layer(text_layer_get_layer(g_digits[4]), GRect(MINS_UNITS_X, 53, 50, 60), GRect(MINS_UNITS_X, -50, 50, 60), 200, 700);
      }

      // Only change minutes tens if its changed
      if((g_state_now[2] != g_state_prev[2]) || (DEBUG_MODE)) {
        util_animate_layer(get_inv_layer(s_beams[2]), GRect(MINS_TENS_X + X_OFFSET, 0, BEAM_WIDTH, 0), GRect(MINS_TENS_X + X_OFFSET, 0, BEAM_WIDTH, BEAM_HEIGHT), 400, 0);
        util_animate_layer(text_layer_get_layer(g_digits[3]), GRect(MINS_TENS_X, 53, 50, 60), GRect(MINS_TENS_X, -50, 50, 60), 200, 700);
      }

      // Only change hours units if its changed
      if((g_state_now[1] != g_state_prev[1]) || (DEBUG_MODE)) {
        util_animate_layer(get_inv_layer(s_beams[1]), GRect(HOURS_UNITS_X + X_OFFSET, 0, BEAM_WIDTH, 0), GRect(HOURS_UNITS_X + X_OFFSET, 0, BEAM_WIDTH, BEAM_HEIGHT), 400, 0);
        util_animate_layer(text_layer_get_layer(g_digits[1]), GRect(HOURS_UNITS_X, 53, 50, 60), GRect(HOURS_UNITS_X, -50, 50, 60), 200, 700);
      }

      // Only change hours tens if its changed
      if((g_state_now[0] != g_state_prev[0]) || (DEBUG_MODE)) {
        util_animate_layer(get_inv_layer(s_beams[0]), GRect(HOUR_TENS_X + X_OFFSET, 0, BEAM_WIDTH, 0), GRect(HOUR_TENS_X + X_OFFSET, 0, BEAM_WIDTH, BEAM_HEIGHT), 400, 0);
        util_animate_layer(text_layer_get_layer(g_digits[0]), GRect(HOUR_TENS_X, 53, 50, 60), GRect(HOUR_TENS_X, -50, 50, 60), 200, 700);
      }
      break;
  }
}

static void bt_handler(bool connected) {
  if(connected) {
    layer_set_hidden(bitmap_layer_get_layer(s_bt_layer), true);
  } else {
    switch (comm_get_setting_value(PERSIST_KEY_BT_VIBE)) {
    case 1:
      vibes_short_pulse();
      break;
    case 2:
      vibes_double_pulse();
      break;
    case 3:
      vibes_long_pulse();
      break;
    }
    layer_set_hidden(bitmap_layer_get_layer(s_bt_layer), false);
  }
}

static void batt_anim_handler(void *context) {
  GRect start = layer_get_frame(get_inv_layer(s_battery_layer));
  GRect finish = GRect(0, 165, 0, 3);

  util_animate_layer(get_inv_layer(s_battery_layer), start, finish, 300, 0);
  s_tapped = false;
}

static void tap_handler(AccelAxisType axis, int32_t direction) {
  BatteryChargeState state = battery_state_service_peek();
  int level = state.charge_percent;
  int width = (int)(float)(((float)level / 100.0F) * (float)144);

  if(!s_tapped) {
    s_tapped = true;

    GRect start = GRect(0, 165, 0, 3);
    GRect finish = GRect(0, 165, width, 3);
    util_animate_layer(get_inv_layer(s_battery_layer), start, finish, 300, 0);

    app_timer_register(3000, batt_anim_handler, NULL);
  }
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);

  // Allocate text layers
  g_digits[0] = util_gen_text_layer(GRect(HOUR_TENS_X, 53, 50, 60), fg_color, GColorClear, true, RESOURCE_ID_FONT_IMAGINE_48, NULL, GTextAlignmentRight);
  layer_add_child(window_layer, text_layer_get_layer(g_digits[0]));

  g_digits[1] = util_gen_text_layer(GRect(HOURS_UNITS_X, 53, 50, 60), fg_color, GColorClear, true, RESOURCE_ID_FONT_IMAGINE_48, NULL, GTextAlignmentRight);
  layer_add_child(window_layer, text_layer_get_layer(g_digits[1]));

  g_digits[2] = util_gen_text_layer(GRect(68, 53, 50, 60), fg_color, GColorClear, true, RESOURCE_ID_FONT_IMAGINE_48, NULL, GTextAlignmentLeft);
  layer_add_child(window_layer, text_layer_get_layer(g_digits[2]));

  g_digits[3] = util_gen_text_layer(GRect(MINS_TENS_X, 53, 50, 60), fg_color, GColorClear, true, RESOURCE_ID_FONT_IMAGINE_48, NULL, GTextAlignmentRight);
  layer_add_child(window_layer, text_layer_get_layer(g_digits[3]));

  g_digits[4] = util_gen_text_layer(GRect(MINS_UNITS_X, 53, 50, 60), fg_color, GColorClear, true, RESOURCE_ID_FONT_IMAGINE_48, NULL, GTextAlignmentRight);
  layer_add_child(window_layer, text_layer_get_layer(g_digits[4]));

  // Allocate inverter layers
  for(int i = 0; i < 4; i++) {
    s_beams[i] = create_inv_layer(GRect(0, 0, BEAM_WIDTH, 0));
    layer_add_child(window_layer, get_inv_layer(s_beams[i]));
  }
  s_seconds_layer = create_inv_layer(GRect(0, 0, 144, 0));
  #ifdef PBL_PLATFORM_BASALT
  inverter_layer_compat_set_colors(fg_color, bg_color);
  #endif
  layer_add_child(window_layer, get_inv_layer(s_seconds_layer));

  // Make sure the face is not blank
  time_t temp = time(NULL);
  struct tm *t = localtime(&temp);
  util_write_time_digits(t);

  // Stop 'all change' on first minute
  for(int i = 0; i < 4; i++) {
    g_state_prev[i] = g_state_now[i];
  }

  // User settings
  g_do_animations = comm_get_setting(PERSIST_KEY_ANIM);
  g_date_layer = util_gen_text_layer(GRect(45, 105, 100, 30), fg_color, GColorClear, true, RESOURCE_ID_FONT_IMAGINE_24, NULL, GTextAlignmentRight);
  if(comm_get_setting(PERSIST_KEY_DATE)) {
    layer_add_child(window_layer, text_layer_get_layer(g_date_layer));
  }
  s_bt_layer = bitmap_layer_create(GRect(59, 140, 27, 26));
#ifdef PBL_PLATFORM_APLITE
  s_bt_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BT);
#elif PBL_PLATFORM_BASALT
  if(comm_get_theme() == THEME_CLASSIC_INVERTED) {
    s_bt_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BT_INV);
  } else {
    s_bt_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BT);
  }
#endif
  bitmap_layer_set_bitmap(s_bt_layer, s_bt_bitmap);
#ifdef PBL_PLATFORM_BASALT
  bitmap_layer_set_compositing_mode(s_bt_layer, GCompOpSet);
#endif
  layer_add_child(window_layer, bitmap_layer_get_layer(s_bt_layer));
  layer_set_hidden(bitmap_layer_get_layer(s_bt_layer), true);
  if(comm_get_setting(PERSIST_KEY_BT)) {
    if(!bluetooth_connection_service_peek()) {
      layer_set_hidden(bitmap_layer_get_layer(s_bt_layer), false);
    }
  }
  s_battery_layer = create_inv_layer(GRect(0, 165, 0, 3));
  if(comm_get_setting(PERSIST_KEY_BATTERY)) {
    layer_add_child(window_layer, get_inv_layer(s_battery_layer));
  }

  // Set time digits now
  util_show_time_digits();

  // Init seconds bar
  int seconds = t->tm_sec;
  if(seconds >= 15 && seconds < 30) {
    util_animate_layer(get_inv_layer(s_seconds_layer), GRect(0, 105, 0, 5), GRect(0, 105, 36, 5), 500, 0);
  } else if(seconds >= 30 && seconds < 45) {
    util_animate_layer(get_inv_layer(s_seconds_layer), GRect(0, 105, 0, 5), GRect(0, 105, 72, 5), 500, 0);
  } else if(seconds >= 45 && seconds < 58) {
    util_animate_layer(get_inv_layer(s_seconds_layer), GRect(0, 105, 0, 5), GRect(0, 105, 108, 5), 500, 0);
  } else if(seconds >= 58) {
    util_animate_layer(get_inv_layer(s_seconds_layer), GRect(0, 105, 0, 5), GRect(0, 105, 144, 5), 500, 1000);
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
    destroy_inv_layer(s_beams[i]);
  }
  destroy_inv_layer(s_seconds_layer);
  destroy_inv_layer(s_battery_layer);
}

static void init() {
  // Prepare to receive app config
  comm_setup();
  comm_first_time_setup();

  // Setup colors
#ifdef PBL_PLATFORM_APLITE
  fg_color = GColorWhite;
  bg_color = GColorBlack;
#elif PBL_PLATFORM_BASALT
  switch(comm_get_theme()) {
    case THEME_CLASSIC:
      fg_color = GColorWhite;
      bg_color = GColorBlack;
      break;
    case THEME_CLASSIC_INVERTED:
      fg_color = GColorBlack;
      bg_color = GColorWhite;
      break;
    case THEME_GREEN:
      fg_color = GColorMintGreen;
      bg_color = GColorIslamicGreen;
      break;
    case THEME_BLUE:
      fg_color = GColorElectricBlue;
      bg_color = GColorBlueMoon;
      break;
    case THEME_RED:
      fg_color = GColorMelon;
      bg_color = GColorRed;
      break;
    case THEME_YELLOW:
      fg_color = GColorPastelYellow;
      bg_color = GColorChromeYellow;
      break;
    case THEME_MIDNIGHT:
      fg_color = GColorWhite;
      bg_color = GColorOxfordBlue;
      break;
    default:
      fg_color = GColorWhite;
      bg_color = GColorBlack;
      break;
  }
#endif

  // Localize
  setlocale(LC_ALL, "");

  // Subscribe to events
  tick_timer_service_subscribe(SECOND_UNIT, handle_tick);
  if(comm_get_setting(PERSIST_KEY_BT)) {
    bluetooth_connection_service_subscribe(bt_handler);
  }
  if(comm_get_setting(PERSIST_KEY_BATTERY)) {
    accel_tap_service_subscribe(tap_handler);
  }

  // Create main window
  s_main_window = window_create();
  window_set_background_color(s_main_window, bg_color);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload
  });
  window_stack_push(s_main_window, true);
}

static void deinit() {
  window_destroy(s_main_window);

  tick_timer_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  accel_tap_service_unsubscribe();
}

int main() {
  init();
  app_event_loop();
  deinit();

  return 0;
}
