#include "comm.h"

static void in_recv_handler(DictionaryIterator *iter, void *context) {
  Tuple *t = dict_read_first(iter);

  while(t) {
    if(t->key != PERSIST_KEY_THEME) {
      // Super settings one liner!
      persist_write_bool(t->key, strcmp(t->value->cstring, "true") == 0 ? true : false);
    } else {
#ifdef PBL_PLATFORM_BASALT
      switch(t->key) {
        case PERSIST_KEY_THEME:
          // Parse theme string
          if(strcmp("classic", t->value->cstring) == 0) {
            persist_write_int(PERSIST_KEY_THEME, THEME_CLASSIC);
          } else if(strcmp("green", t->value->cstring) == 0) {
            persist_write_int(PERSIST_KEY_THEME, THEME_GREEN);
          } else if(strcmp("blue", t->value->cstring) == 0) {
            persist_write_int(PERSIST_KEY_THEME, THEME_BLUE);
          }
          break;
        default: 
          break;
      }
#endif
    }

    t = dict_read_next(iter);
  }

  vibes_short_pulse();
}

static void in_failed_handler(AppMessageResult reason, void *context) {
  util_interpret_message_result(reason);
}

void comm_first_time_setup() {
  if(!persist_exists(PERSIST_KEY_DATE)) {
    persist_write_bool(PERSIST_KEY_DATE, false);
  }
  if(!persist_exists(PERSIST_KEY_ANIM)) {
    persist_write_bool(PERSIST_KEY_ANIM, true);
  }
  if(!persist_exists(PERSIST_KEY_BT)) {
    persist_write_bool(PERSIST_KEY_BT, true);
  }
  if(!persist_exists(PERSIST_KEY_BATTERY)) {
    persist_write_bool(PERSIST_KEY_BATTERY, false);
  }
  if(!persist_exists(PERSIST_KEY_HOURLY)) {
    persist_write_bool(PERSIST_KEY_HOURLY, false);
  }
}

void comm_setup() {
  app_message_register_inbox_dropped(in_failed_handler);
  app_message_register_inbox_received(in_recv_handler);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

bool comm_get_setting(int key) {
  return persist_read_bool(key);
}

int comm_get_theme() {
  return persist_read_int(PERSIST_KEY_THEME);
}