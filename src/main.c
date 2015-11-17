#include <pebble.h>

#include "windows/main_window.h"

#include "modules/data.h"
#include "modules/comm.h"

static void tick_handler(struct tm* tick_time, TimeUnits changed) {
  main_window_update_time(tick_time);
}

static void init() {
  data_init();
  comm_init(128, 128, false);
  setlocale(LC_ALL, "");

  main_window_push();

  tick_timer_service_subscribe(data_get_boolean_setting(DataKeyAnimations)
    ? SECOND_UNIT : MINUTE_UNIT, tick_handler);
}

static void deinit() {
  data_deinit();
  comm_deinit();
}

int main() {
  init();
  app_event_loop();
  deinit();
}
