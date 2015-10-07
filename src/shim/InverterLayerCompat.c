#ifdef PBL_SDK_3
#include "InverterLayerCompat.h"

static GColor s_fg_color, s_bg_color;

static GColor get_pixel(GBitmapDataRowInfo info, GPoint pixel) {
  if(pixel.x >= info.min_x && pixel.x < info.max_x) {
    return (GColor) { .argb = info.data[pixel.x - info.min_x] };
  } else {
    return GColorRed;
  }
}

static void set_pixel(GBitmapDataRowInfo info, GPoint pixel, GColor color) {
  if(pixel.x >= info.min_x && pixel.x < info.max_x) {
    memset(&info.data[pixel.x - info.min_x], (uint8_t)color.argb, 1);
  }
}

static void layer_update_proc(Layer *layer, GContext *ctx) {
  // Use framebuffer to emulate inverstion
  GBitmap *fb = graphics_capture_frame_buffer(ctx);
  GSize size = gbitmap_get_bounds(fb).size;
  GRect frame = layer_get_frame(layer);

  for(int y = frame.origin.y; y < frame.origin.y + frame.size.h; y++) {
    GBitmapDataRowInfo info = gbitmap_get_data_row_info(fb, y);

    for(int x = info.min_x; x < info.max_x; x++) {
      if(gcolor_equal(get_pixel(info, GPoint(x, y)), s_fg_color)) {
        // Invert foreground to background
        set_pixel(info, GPoint(x, y), s_bg_color);
      } else if(gcolor_equal(get_pixel(info, GPoint(x, y)), s_bg_color)) {
        // Invert background to foreground
        set_pixel(info, GPoint(x, y), s_fg_color);
      }
    }
  }

  // Finally
  graphics_release_frame_buffer(ctx, fb);
}

InverterLayerCompat *inverter_layer_compat_create(GRect bounds) {
  InverterLayerCompat *this = (InverterLayerCompat*)malloc(sizeof(InverterLayerCompat));
  this->layer = layer_create(bounds);
  layer_set_update_proc(this->layer, layer_update_proc);

  return this;
}

void inverter_layer_compat_set_colors(GColor fg, GColor bg) {
  s_fg_color = fg;
  s_bg_color = bg;
}

void inverter_layer_compat_destroy(InverterLayerCompat *this) {
  layer_destroy(this->layer);
  free(this);
}

Layer* inverter_layer_compat_get_layer(InverterLayerCompat *this) {
  return this->layer;
}

#endif