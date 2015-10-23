#include "util.h"

/**
  * Function to get time digits into buffers
  */
void util_write_time_digits(struct tm *t) {
  // Hour string
  if(clock_is_24h_style()) {
    strftime(g_time_buffer, sizeof("XX:XX"), "%H:%M", t);
  } else {
    strftime(g_time_buffer, sizeof("XX:XX"), "%I:%M", t);
  }
  
  // Get digits
  g_state_now[3] = g_time_buffer[4] - '0';  //Conv to int from char
  g_state_now[2] = g_time_buffer[3] - '0';
  g_state_now[1] = g_time_buffer[1] - '0';
  g_state_now[0] = g_time_buffer[0] - '0';

  strftime(g_date_buffer, sizeof(g_date_buffer), "%a %d", t);   //Sun 01
}

/**
  * Function to set the time and date digits on the TextLayers
  */
void util_show_time_digits() {  
  if(!DEBUG_MODE) {
    // Include null chars
    static char s_chars[4][2] = {"1", "2", "3", "4"};
    s_chars[0][0] = g_time_buffer[4];
    s_chars[1][0] = g_time_buffer[3];
    s_chars[2][0] = g_time_buffer[1];
    s_chars[3][0] = g_time_buffer[0];

    // Set digits in TextLayers
    text_layer_set_text(g_digits[0], s_chars[3]);
    text_layer_set_text(g_digits[1], s_chars[2]);
    text_layer_set_text(g_digits[2], ":");
    text_layer_set_text(g_digits[3], s_chars[1]);
    text_layer_set_text(g_digits[4], s_chars[0]);

    // Set date
    text_layer_set_text(g_date_layer, g_date_buffer);
  } else {
    // Test digits
    text_layer_set_text(g_digits[0], "2");
    text_layer_set_text(g_digits[1], "3");
    text_layer_set_text(g_digits[2], ":");
    text_layer_set_text(g_digits[3], "5");
    text_layer_set_text(g_digits[4], "9");
  }
}

/**
  * Function to predict digit changes to make the digit change mechanic fire correctly.
  * If the values change at seconds == 0, then the animations depending on g_state_now[3] != s_state_prev[3]
  * will not fire! 
  * The solution is to advance the ones about to change pre-emptively
  */
void util_predict_next_change(struct tm *t) {
  // These will be for the previous minute at 59 seconds
  util_write_time_digits(t);
   
  // Fix hours tens
  if(
     ((g_state_now[0] == 0) && (g_state_now[1] == 9) && (g_state_now[2] == 5) && (g_state_now[3] == 9))   //09:59 --> 10:00
  || ((g_state_now[0] == 1) && (g_state_now[1] == 9) && (g_state_now[2] == 5) && (g_state_now[3] == 9))   //19:59 --> 20:00
  || ((g_state_now[0] == 2) && (g_state_now[1] == 3) && (g_state_now[2] == 5) && (g_state_now[3] == 9))   //23:59 --> 00:00
  ) {
    g_state_now[0]++;
  }
 
  // Fix hours units
  if((g_state_now[2] == 5) && (g_state_now[3] == 9)) {
    g_state_now[1]++;
  }
 
  // Fix minutes tens
  if(g_state_now[3] == 9) { //Minutes Tens about to change
    g_state_now[2]++;
  }
     
  // Finally fix minutes units
  g_state_now[3]++;
}

/**
 * Initialize a text layer
 */
TextLayer* util_gen_text_layer(GRect location, GColor colour, GColor background, bool custom_font, int custom_res_id, const char *res_id, GTextAlignment alignment) {
  TextLayer *layer = text_layer_create(location);
  text_layer_set_text_color(layer, colour);
  text_layer_set_background_color(layer, background);
  if(custom_font == true)
  {
    ResHandle handle = resource_get_handle(custom_res_id);
    text_layer_set_font(layer, fonts_load_custom_font(handle));
  }
  else
  {
    text_layer_set_font(layer, fonts_get_system_font(res_id));
  }
  text_layer_set_text_alignment(layer, alignment);

  return layer;
}

/*
 * Internal Animation disposal
 */
static void on_animation_stopped(Animation *anim, bool finished, void *context) {
#if defined(PBL_SDK_2)
  property_animation_destroy((PropertyAnimation*) anim);
#endif
}

/*
 * Animate a layer with duration and delay
 */
void util_animate_layer(Layer *layer, GRect start, GRect finish, int duration, int delay) {
  if(!g_do_animations) {
    return;
  }

  PropertyAnimation *anim = property_animation_create_layer_frame(layer, &start, &finish);
  
  animation_set_duration((Animation*) anim, duration);
  animation_set_delay((Animation*) anim, delay);
  
  AnimationHandlers handlers = {
    .stopped = (AnimationStoppedHandler) on_animation_stopped
  };
  animation_set_handlers((Animation*) anim, handlers, NULL);
  
  animation_schedule((Animation*) anim);
}

void util_interpret_message_result(AppMessageResult app_message_error) {
  if(app_message_error == APP_MSG_OK)
  {
    APP_LOG(APP_LOG_LEVEL_ERROR, "APP_MSG_OK");
  } 

  else if(app_message_error == APP_MSG_SEND_TIMEOUT)
  {
    APP_LOG(APP_LOG_LEVEL_ERROR, "APP_MSG_SEND_TIMEOUT");
  } 

  else if(app_message_error == APP_MSG_SEND_REJECTED)
  {
    APP_LOG(APP_LOG_LEVEL_ERROR, "APP_MSG_SEND_REJECTED");
  }

  else if(app_message_error == APP_MSG_NOT_CONNECTED)
  {
    APP_LOG(APP_LOG_LEVEL_ERROR, "APP_MSG_NOT_CONNECTED");
  }

  else if(app_message_error == APP_MSG_APP_NOT_RUNNING)
  {
    APP_LOG(APP_LOG_LEVEL_ERROR, "APP_MSG_APP_NOT_RUNNING");
  }

  else if(app_message_error == APP_MSG_INVALID_ARGS)
  {
    APP_LOG(APP_LOG_LEVEL_ERROR, "APP_MSG_INVALID_ARGS");
  }

  else if(app_message_error == APP_MSG_BUSY)
  {
    APP_LOG(APP_LOG_LEVEL_ERROR, "APP_MSG_BUSY");
  }

  else if(app_message_error == APP_MSG_BUFFER_OVERFLOW)
  {
    APP_LOG(APP_LOG_LEVEL_ERROR, "APP_MSG_BUFFER_OVERFLOW");
  }

  else if(app_message_error == APP_MSG_ALREADY_RELEASED)
  {
    APP_LOG(APP_LOG_LEVEL_ERROR, "APP_MSG_ALREADY_RELEASED");
  }

  else if(app_message_error == APP_MSG_CALLBACK_ALREADY_REGISTERED)
  {
    APP_LOG(APP_LOG_LEVEL_ERROR, "APP_MSG_CALLBACK_ALREADY_REGISTERED");
  }

  else if(app_message_error == APP_MSG_CALLBACK_NOT_REGISTERED)
  {
    APP_LOG(APP_LOG_LEVEL_ERROR, "APP_MSG_CALLBACK_NOT_REGISTERED");
  }

  else if(app_message_error == APP_MSG_OUT_OF_MEMORY)
  {
    APP_LOG(APP_LOG_LEVEL_ERROR, "APP_MSG_OUT_OF_MEMORY");
  }
}
