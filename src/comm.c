#include "comm.h"

static void in_recv_handler(DictionaryIterator *iter, void *context) {
  Tuple *t = dict_read_first(iter);

  while(t) {
    // Super settings one liner!
    persist_write_bool(t->key, strcmp(t->value->cstring, "true") == 0 ? true : false);

    t = dict_read_next(iter);
  }

  vibes_short_pulse();
}

static void in_failed_handler(AppMessageResult reason, void *context) {
  interpret_message_result(reason);
}

void comm_setup() {
  app_message_register_inbox_dropped(in_failed_handler);
  app_message_register_inbox_received(in_recv_handler);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

  APP_LOG(APP_LOG_LEVEL_DEBUG, "comm_setup()");
}

bool comm_get_setting(int key) {
  return persist_read_bool(key);
}