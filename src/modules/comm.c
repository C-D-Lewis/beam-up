#include "comm.h"

static void inbox_received_handler(DictionaryIterator *iter, void *context) {

}

void comm_init(uint32_t inbox, uint32_t outbox, bool maximum) {
  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(
    maximum ? app_message_inbox_size_maximum() : inbox,
    maximum ? app_message_outbox_size_maximum() : outbox);
}

void comm_deinit() {
  app_message_deregister_callbacks();
}
