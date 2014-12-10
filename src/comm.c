#include "comm.h"

static void in_recv_handler(DictionaryIterator *iter, void *context) {
  Tuple *t = dict_read_first(iter);

  while(t) {
    // Super settings one liner!
    persist_write_bool(t->key, strcmp(t->value->cstring, "true") == 0 ? true : false);

    t = dict_read_next(iter);
  }
}

void comm_setup() {
  app_message_register_inbox_received(in_recv_handler);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

bool comm_get_setting(int key) {
  return persist_read_bool(key);
}