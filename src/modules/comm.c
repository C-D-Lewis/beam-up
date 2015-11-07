#include "comm.h"

// TODO Can collapse this, but it wouldn't be a good pattern to use
static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  Tuple *date_tuple = dict_find(iter, DataKeyDate);
  if(date_tuple) {
    data_set_boolean_setting(DataKeyDate, strcmp(date_tuple->value->cstring, "true") == 0);
  }

  Tuple *anim_tuple = dict_find(iter, DataKeyAnimations);
  if(anim_tuple) {
    data_set_boolean_setting(DataKeyAnimations, strcmp(anim_tuple->value->cstring, "true") == 0);
  }

  Tuple *bt_tuple = dict_find(iter, DataKeyBTIndicator);
  if(bt_tuple) {
    data_set_boolean_setting(DataKeyBTIndicator, strcmp(bt_tuple->value->cstring, "true") == 0);
  }

  Tuple *hourly_tuple = dict_find(iter, DataKeyHourlyVibration);
  if(hourly_tuple) {
    data_set_boolean_setting(DataKeyHourlyVibration, strcmp(hourly_tuple->value->cstring, "true") == 0);
  }

#if defined(PBL_SDK_3)
  Tuple *fg_tuple = dict_find(iter, DataKeyForegroundColor);
  if(fg_tuple) {
    data_set_foreground_color((GColor){ .argb = fg_tuple->value->int32 });
  }

  Tuple *bg_tuple = dict_find(iter, DataKeyBackgroundColor);
  if(bg_tuple) {
    data_set_background_color((GColor){ .argb = bg_tuple->value->int32 });
  }
#endif

  // Exit to reload settings
  window_stack_pop_all(true);
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
