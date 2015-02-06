#include "comm.h"

static void in_recv_handler(DictionaryIterator *iter, void *context) {
  Tuple *t = dict_read_first(iter);

  while(t) {
    if (t->key < PERSIST_MAX_BOOLEANS) {
      // Super settings one liner!
      persist_write_bool(t->key, strcmp(t->value->cstring, "true") == 0 ? true : false);
    } else {
      // I really hate mysef for breaking this ^ feature :)
      persist_write_int(t->key, t->value->int32);
    }
    
    t = dict_read_next(iter);
  }

  vibes_short_pulse();
}

static void in_failed_handler(AppMessageResult reason, void *context) {
  util_interpret_message_result(reason);
}

void comm_setup() {
  app_message_register_inbox_dropped(in_failed_handler);
  app_message_register_inbox_received(in_recv_handler);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

bool comm_get_setting(int key) {
  return persist_read_bool(key);
}

int comm_get_setting_value(int key) {
  return persist_read_int(key);
}
