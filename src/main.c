#include "pebble.h"

Window *window;
static GBitmap *image;
BitmapLayer *die_layer;

// Called once per second
static void handle_second_tick(struct tm* tick_time, TimeUnits units_changed) {
  
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  
  GRect bounds = image->bounds;
  
  int minute = t->tm_min;
  int min6 = minute/10;
  int fix = min6*144;
  
  layer_set_frame(bitmap_layer_get_layer(die_layer), GRect(-fix, 0, 864, 154));
}

// Handle the start-up of the app
static void do_init() {

  // Create our app's base window
  window = window_create();
  window_stack_push(window, true);
  window_set_background_color(window, GColorBlack);
  
  image = gbitmap_create_with_resource(RESOURCE_ID_DICE_FACE);
  
  die_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_bitmap(die_layer, image);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(die_layer));

  Layer *root_layer = window_get_root_layer(window);
  GRect frame = layer_get_frame(root_layer);

  time_t now = time(NULL);
  struct tm *current_time = localtime(&now);
  handle_second_tick(current_time, MINUTE_UNIT);

  tick_timer_service_subscribe(MINUTE_UNIT, &handle_second_tick);
}

static void do_deinit() {
  tick_timer_service_unsubscribe();
  window_destroy(window);
  
}

// The main event/run loop for our app
int main(void) {
  do_init();
  app_event_loop();
  do_deinit();
}