#include "util.h"

/**
  * Function to get time digits into buffers
  */
void write_time_digits(struct tm *t) {
  // Hour string
  if(clock_is_24h_style()) {
    strftime(s_time_buffer, sizeof("XX:XX"), "%H:%M", t);
  } else {
    strftime(s_time_buffer, sizeof("XX:XX"), "%I:%M", t);
  }
  
  // Get digits
  s_state_now[3] = s_time_buffer[4] - '0';  //Conv to int from char
  s_state_now[2] = s_time_buffer[3] - '0';
  s_state_now[1] = s_time_buffer[1] - '0';
  s_state_now[0] = s_time_buffer[0] - '0';
}

/**
  * Function to set the time and date digits on the TextLayers
  */
void show_time_digits() {  
  if(!DEBUG_MODE) {
    // Include null chars
    static char chars[4][2] = {"1", "2", "3", "4"};
    chars[0][0] = s_time_buffer[4];
    chars[1][0] = s_time_buffer[3];
    chars[2][0] = s_time_buffer[1];
    chars[3][0] = s_time_buffer[0];

    // Set digits in TextLayers
    text_layer_set_text(s_digits[0], chars[3]);
    text_layer_set_text(s_digits[1], chars[2]);
    text_layer_set_text(s_digits[2], ":");
    text_layer_set_text(s_digits[3], chars[1]);
    text_layer_set_text(s_digits[4], chars[0]);
  } else {
    // Test digits
    text_layer_set_text(s_digits[0], "2");
    text_layer_set_text(s_digits[1], "3");
    text_layer_set_text(s_digits[2], ":");
    text_layer_set_text(s_digits[3], "5");
    text_layer_set_text(s_digits[4], "9");
  }
}

/**
  * Function to predict digit changes to make the digit change mechanic fire correctly.
  * If the values change at seconds == 0, then the animations depending on s_state_now[3] != s_state_prev[3]
  * will not fire! 
  * The solution is to advance the ones about to change pre-emptively
  */
void predict_next_change(struct tm *t) {
  // These will be for the previous minute at 59 seconds
  write_time_digits(t);
   
  // Fix hours tens
  if(
     ((s_state_now[0] == 0) && (s_state_now[1] == 9) && (s_state_now[2] == 5) && (s_state_now[3] == 9))   //09:59 --> 10:00
  || ((s_state_now[0] == 1) && (s_state_now[1] == 9) && (s_state_now[2] == 5) && (s_state_now[3] == 9))   //19:59 --> 20:00
  || ((s_state_now[0] == 2) && (s_state_now[1] == 3) && (s_state_now[2] == 5) && (s_state_now[3] == 9))   //23:59 --> 00:00
  ) {
    s_state_now[0]++;
  }
 
  // Fix hours units
  if((s_state_now[2] == 5) && (s_state_now[3] == 9)) {
    s_state_now[1]++;
  }
 
  // Fix minutes tens
  if(s_state_now[3] == 9) { //Minutes Tens about to change
    s_state_now[2]++;
  }
     
  // Finally fix minutes units
  s_state_now[3]++;
}

/**
 * Initialize a text layer
 */
TextLayer* gen_text_layer(GRect location, GColor colour, GColor background, bool custom_font, int custom_res_id, const char *res_id, GTextAlignment alignment) {
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
  property_animation_destroy((PropertyAnimation*) anim);
}

/*
 * Animate a layer with duration and delay
 */
void animate_layer(Layer *layer, GRect start, GRect finish, int duration, int delay) {
  PropertyAnimation *anim = property_animation_create_layer_frame(layer, &start, &finish);
  
  animation_set_duration((Animation*) anim, duration);
  animation_set_delay((Animation*) anim, delay);
  
  AnimationHandlers handlers = {
    .stopped = (AnimationStoppedHandler) on_animation_stopped
  };
  animation_set_handlers((Animation*) anim, handlers, NULL);
  
  animation_schedule((Animation*) anim);
}

void interpret_message_result(AppMessageResult app_message_error) {
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
