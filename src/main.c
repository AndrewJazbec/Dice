#include "pebble.h"

Window *window;
static GBitmap *image;
BitmapLayer *die_layer;
TextLayer *time_layer;
const int i_h = 154, i_w = 864;
int pub_fix = 0;

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
  static char time_text[] = "00:00"; // Needs to be static because it's used by the system later.
  
  strftime(time_text, sizeof(time_text), clock_is_24h_style()?"%T":"%I:%M", tick_time);
  text_layer_set_text(time_layer, time_text);
  
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
    
  int minute = t->tm_min;
  int min6 = minute/10;
  pub_fix = -(min6*144);
  
  layer_set_frame(bitmap_layer_get_layer(die_layer), GRect(pub_fix, 0, i_w, i_h));
}

void tap_handler(AccelAxisType accel, int32_t direction) {
  
  GRect start = GRect(pub_fix, 0, i_w, i_h);
  GRect finish = GRect(pub_fix, -120, i_w, i_h);
  animate_layer(bitmap_layer_get_layer(die_layer), &start, &finish, 700, 0);
  
  GRect start1 = GRect(pub_fix, -120, i_w, i_h);
  GRect finish1 = GRect(pub_fix, 0, i_w, i_h);
  animate_layer(bitmap_layer_get_layer(die_layer), &start1, &finish1, 700, 5000);
  
}

void window_load(Window *w){
  Layer *root_layer = window_get_root_layer(w);
  
  time_layer = text_layer_create(GRect(0, 100, 144, 100));
  text_layer_set_text_color(time_layer, GColorWhite);
  text_layer_set_background_color(time_layer, GColorClear);
  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
  text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  layer_add_child(root_layer, text_layer_get_layer(time_layer));
  
  image = gbitmap_create_with_resource(RESOURCE_ID_DICE_FACE);
  
  die_layer = bitmap_layer_create(GRect(-144, 0, 144, 168));
  bitmap_layer_set_bitmap(die_layer, image);
  layer_add_child(root_layer, bitmap_layer_get_layer(die_layer));

  //Layer *root_layer = window_get_root_layer(window);
  GRect frame = layer_get_frame(root_layer);

  time_t now = time(NULL);
  struct tm *current_time = localtime(&now);
  handle_second_tick(current_time, MINUTE_UNIT);
}

void window_unload(Window *w){
  gbitmap_destroy(image);
  text_layer_destroy(time_layer);
  bitmap_layer_destroy(die_layer);
}

void init() {
  window = window_create();
  window_set_background_color(window, GColorBlack);
  window_set_window_handlers(window, (WindowHandlers){
    .load = window_load,
    .unload = window_unload,
  });
  accel_tap_service_subscribe(tap_handler);
  tick_timer_service_subscribe(MINUTE_UNIT, &handle_second_tick);
  window_stack_push(window, true);
}

static void deinit() {
  tick_timer_service_unsubscribe();
  accel_tap_service_unsubscribe();
  text_layer_destroy(time_layer);
  window_destroy(window);
  
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}