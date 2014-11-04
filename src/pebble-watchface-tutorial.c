#include <pebble.h>
static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static TextLayer *s_day_layer;
static GFont s_time_font;
static GFont s_date_font;
static GFont s_day_font;
static BitmapLayer *s_background_layer;
static RotBitmapLayer *s_seconds_layer;
static GBitmap *s_background_bitmap;
static GBitmap *s_seconds_bitmap;
static GPoint center = { 72, 84 };
static int32_t sec_angle;

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char buffer[] = "00:00";
  static char date_buffer[] = "00.00";
  static char day_buffer[] = "Thu";

  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    // Use 24 hour format
    strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    // Use 12 hour format
    strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
  }

  // Write the current date into the date buffer
  strftime(date_buffer, sizeof("00.00"), "%m.%d", tick_time);

  // Write the current day into the day buffer
  strftime(day_buffer, sizeof("Thu"), "%a", tick_time);

  // Get second angle
  sec_angle = TRIG_MAX_ANGLE * tick_time->tm_sec / 60;

  // Display this time and date on the TextLayer
  text_layer_set_text(s_time_layer, buffer);
  text_layer_set_text(s_date_layer, date_buffer);
  text_layer_set_text(s_day_layer, day_buffer);

  // Seconds layer
  GRect r;
  r = layer_get_frame((Layer *)s_seconds_layer);
  r.origin.x = 72 - r.size.w/2;
  r.origin.y = 84 - r.size.h/2;
  layer_set_frame((Layer *)s_seconds_layer, r);
  rot_bitmap_layer_set_angle(s_seconds_layer, sec_angle);
  rot_bitmap_set_compositing_mode(s_seconds_layer, GCompOpOr);
  rot_bitmap_set_src_ic(s_seconds_layer, center);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
   update_time();
}

static void main_window_load(Window *window) {
  // Create new font
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_PLAY_FONT_REGULAR_35));
  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_PLAY_FONT_BOLD_20));
  s_day_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_PLAY_FONT_REGULAR_16));

  // Create GBitmap, then set to created BitmapLayer
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_MAIN_WATCHFACE);
  s_background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_layer));

  
  // Create GBitmap, then set to created RotBitmapLayer
  s_seconds_bitmap = gbitmap_create_with_resource(RESOURCE_ID_SECONDS_DIAL);
  s_seconds_layer = rot_bitmap_layer_create(s_seconds_bitmap);
  layer_add_child(window_get_root_layer(window), (Layer*)s_seconds_layer);
  
  
  // Create time TextLayer
  s_time_layer = text_layer_create(GRect(0, 60, 144, 50));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  // Apply to TextLayer
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));


  // Create date TextLayer
  s_date_layer = text_layer_create(GRect(0, 144, 144, 26));
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorWhite);
  // Show values
  // static char rbuff[0x100];
  // r = layer_get_frame((Layer *)s_seconds_layer);
  // snprintf(rbuff, sizeof(rbuff), "%d,%d,%d", r.origin.y,r.size.w,r.size.h);
  //text_layer_set_text(s_date_layer,rbuff);
  // Apply to TextLayer
  text_layer_set_font(s_date_layer, s_date_font);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));

  // Create day TextLayer
  s_day_layer = text_layer_create(GRect(0, 0, 144, 26));
  text_layer_set_background_color(s_day_layer, GColorClear);
  text_layer_set_text_color(s_day_layer, GColorWhite);
  //text_layer_set_text(s_date_layer,"Mon");
  // Apply to TextLayer
  text_layer_set_font(s_day_layer, s_day_font);
  text_layer_set_text_alignment(s_day_layer, GTextAlignmentCenter);
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_day_layer));

  // Make sure the time is displayed from the start
  update_time();
}

static void main_window_unload(Window *window) {
    // Destroy TextLayer
    text_layer_destroy(s_time_layer);
	text_layer_destroy(s_date_layer);
	text_layer_destroy(s_day_layer);
  
    // Unload GFont
    fonts_unload_custom_font(s_time_font);
    
    // Destroy GBitmap and BitmapLayer
    gbitmap_destroy(s_background_bitmap);
    bitmap_layer_destroy(s_background_layer);
    
    // Destroy GBitmap and RotBitmapLayer
    gbitmap_destroy(s_seconds_bitmap);
    rot_bitmap_layer_destroy(s_seconds_layer);
    
}

static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Register with TickTimerService
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
}

static void deinit() {
    // Destroy Window
    window_destroy(s_main_window);
}

int main(void) {
  // Initialize main Window
  init();

  // Start event loop
  app_event_loop();

  // Deinit main Window
  deinit();
}