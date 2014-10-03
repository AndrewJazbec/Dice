#include "pebble.h"

Window *window;
static GBitmap *image;
BitmapLayer *die_layer;
TextLayer *time_layer; 

//test
void on_animation_stopped(Animation *anim, bool finished, void *context)
{
    //Free the memoery used by the Animation
    property_animation_destroy((PropertyAnimation*) anim);
}
 
void animate_layer(Layer *layer, GRect *start, GRect *finish, int duration, int delay)
{
    //Declare animation
    PropertyAnimation *anim = property_animation_create_layer_frame(layer, start, finish);
 
    //Set characteristics
    animation_set_duration((Animation*) anim, duration);
    animation_set_delay((Animation*) anim, delay);
 
    //Set stopped handler to free memory
    AnimationHandlers handlers = {
        //The reference to the stopped handler is the only one in the array
        .stopped = (AnimationStoppedHandler) on_animation_stopped
    };
    animation_set_handlers((Animation*) anim, handlers, NULL);
 
    //Start animation!
    animation_schedule((Animation*) anim);
}

// Called once per second
static void handle_second_tick(struct tm* tick_time, TimeUnits units_changed) {
  
  strftime(time_text, sizeof(time_text), clock_is_24h_style()?"""%M", tick_time);
  text_layer_set_text(time_layer, time_text);
  
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  
  GRect bounds = image->bounds;
  
  int minute = t->tm_min;
  int min6 = minute/10;
  int fix = min6*144;
  
  layer_set_frame(bitmap_layer_get_layer(die_layer), GRect(-fix, 0, 864, 154));
}

void tap_handler(AccelAxisType accel, int32_t direction) {
  
  GRect start = GRect(110, 134, 144, 34);
  GRect finish = GRect(0, 134, 144, 34);
  animate_layer(text_layer_get_layer(battery_layer), &start, &finish, 200, 500);
  
  GRect start1 = GRect(0, 134, 144, 34);
  GRect finish1 = GRect(110, 134, 144, 34);
  animate_layer(text_layer_get_layer(battery_layer), &start1, &finish1, 200, 1700);
  
}

// Handle the start-up of the app
static void do_init() {

  // Create our app's base window
  window = window_create();
  window_stack_push(window, true);
  window_set_background_color(window, GColorBlack);
  
  image = gbitmap_create_with_resource(RESOURCE_ID_DICE_FACE);
  
  time_layer = text_layer_create(GRect(0, 60, frame.size.w /* width */, 100/* height */));
  text_layer_set_text_color(time_layer, GColorWhite);
  text_layer_set_background_color(time_layer, GColorClear);
  text_layer_set_font(time_layer, fonts_load_custom_font(font_handle));
  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
  
  die_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_bitmap(die_layer, image);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(die_layer));

  Layer *root_layer = window_get_root_layer(window);
  GRect frame = layer_get_frame(root_layer);

  time_t now = time(NULL);
  struct tm *current_time = localtime(&now);
  handle_second_tick(current_time, MINUTE_UNIT);

  tick_timer_service_subscribe(MINUTE_UNIT, &handle_second_tick);
  
  layer_add_child(root_layer, text_layer_get_layer(time_layer));
  
  accel_tap_service_subscribe(tap_handler);
}

static void do_deinit() {
  tick_timer_service_unsubscribe();
  text_layer_destroy(time_layer);
  window_destroy(window);
  
}

// The main event/run loop for our app
int main(void) {
  do_init();
  app_event_loop();
  do_deinit();
}