#include <pebble.h>

#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS 1

static Window *s_main_window;

static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static TextLayer *s_day_layer;
static TextLayer *s_charged_layer;
static TextLayer *s_temp_layer;

static GFont s_time_font;
static GFont s_date_font;
static GFont s_day_font;
static GFont s_charged_font;

static BitmapLayer *s_battery_layer;
static BitmapLayer *s_bottom_layer;
static BitmapLayer *s_charging_layer;

static RotBitmapLayer *s_battery_mask_layer;
static RotBitmapLayer *s_seconds_layer;

static GBitmap *s_battery_bitmap;
static GBitmap *s_battery_mask_bitmap;
static GBitmap *s_bottom_bitmap;
static GBitmap *s_seconds_bitmap;
static GBitmap *s_charging_bitmap;

static GRect r;
static GPoint center = { 72, 84 };

static int32_t sec_angle;
static int32_t bat_angle;

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
	r = layer_get_frame((Layer *)s_seconds_layer);
	r.origin.x = 72 - r.size.w/2;
	r.origin.y = 84 - r.size.h/2;
	layer_set_frame((Layer *)s_seconds_layer, r);
	rot_bitmap_layer_set_angle(s_seconds_layer, sec_angle);
	rot_bitmap_set_compositing_mode(s_seconds_layer, GCompOpOr);
	rot_bitmap_set_src_ic(s_seconds_layer, center);	
}

static void update_bluetooth(bool connected){
	if(connected){
		//text_layer_set_text(s_temp_layer,"C");
	}else{
		//text_layer_set_text(s_temp_layer, "x");
	}
}

static void update_battery(BatteryChargeState charge){
	
	static char charge_now[32];
	int charge_percent = charge.charge_percent;
	bool is_charging = charge.is_charging;
	bool is_plugged = charge.is_plugged;
	snprintf(charge_now, sizeof(charge_now), "%d%%", charge_percent);
	
	// Battery charge
	// 100% = 0 deg = 0; 0% = 123 deg = 0.34
	bat_angle = (TRIG_MAX_ANGLE * ((charge_percent-100)/100) * 34 / 100) ;
	GRect rp;
	rp = layer_get_frame((Layer *)s_battery_mask_layer);
	rp.size.w = 144;
	rp.size.h = 168;
	rp.origin.x = 72 - rp.size.w/2;
	rp.origin.y = 84 - rp.size.h/2;
	layer_set_frame((Layer *)s_battery_mask_layer, rp);
	rot_bitmap_layer_set_angle(s_battery_mask_layer, bat_angle);
	rot_bitmap_set_compositing_mode(s_battery_mask_layer, GCompOpAnd);
	rot_bitmap_set_src_ic(s_battery_mask_layer, center);
	
	/*
	// Show charging
	if(is_charging) {
		layer_set_hidden(bitmap_layer_get_layer(s_charging_layer), false);
		layer_set_hidden(text_layer_get_layer(s_charged_layer), true);
	} else if(is_plugged && charge_percent == 100) {
		layer_set_hidden(bitmap_layer_get_layer(s_charging_layer), true);
		text_layer_set_text(s_charged_layer, "Charged");
		layer_set_hidden(text_layer_get_layer(s_charged_layer), false);
	} else {
		layer_set_hidden(bitmap_layer_get_layer(s_charging_layer), true);
		text_layer_set_text(s_charged_layer, charge_now);
		layer_set_hidden(text_layer_get_layer(s_charged_layer), false);
	}*/
}

static void main_window_load(Window *window) {
  	// Custom fonts
	s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_PLAY_FONT_REGULAR_35));
	s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_PLAY_FONT_BOLD_20));
	s_day_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_PLAY_FONT_REGULAR_16));
	s_charged_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_PLAY_FONT_REGULAR_14));


	// BATTERY BAR
	s_battery_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_BAR);
	s_battery_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
	bitmap_layer_set_bitmap(s_battery_layer, s_battery_bitmap);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_battery_layer));
	
	
	// BATTERY MASK
	s_battery_mask_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_MASK);
	s_battery_mask_layer = rot_bitmap_layer_create(s_battery_mask_bitmap);
	layer_add_child(window_get_root_layer(window), (Layer*)s_battery_mask_layer);
	
	
	// CHARGING
	s_charging_bitmap = gbitmap_create_with_resource(RESOURCE_ID_CHARGING);
	s_charging_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
	bitmap_layer_set_bitmap(s_charging_layer, s_charging_bitmap);
	bitmap_layer_set_compositing_mode(s_charging_layer, GCompOpOr);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_charging_layer));
	layer_set_hidden(bitmap_layer_get_layer(s_charging_layer), true);
	
	
	// CHARGED
	s_charged_layer = text_layer_create(GRect(0, 50, 144, 26));
	text_layer_set_background_color(s_charged_layer, GColorClear);
	text_layer_set_text_color(s_charged_layer, GColorWhite);
	text_layer_set_font(s_charged_layer, s_charged_font);
	text_layer_set_text_alignment(s_charged_layer, GTextAlignmentCenter);
	//text_layer_set_text(s_charged_layer, "");
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_charged_layer));
	//layer_set_hidden(text_layer_get_layer(s_charged_layer), true);
	
	
	// BOTTOM BAR
	s_bottom_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BOTTOM_BAR);
	s_bottom_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
	bitmap_layer_set_bitmap(s_bottom_layer, s_bottom_bitmap);
	bitmap_layer_set_compositing_mode(s_bottom_layer, GCompOpOr);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_bottom_layer));


	// SECONDS DIAL
	s_seconds_bitmap = gbitmap_create_with_resource(RESOURCE_ID_SECONDS_DIAL);
	s_seconds_layer = rot_bitmap_layer_create(s_seconds_bitmap);
	layer_add_child(window_get_root_layer(window), (Layer*)s_seconds_layer);


	// TIME TEXT
	s_time_layer = text_layer_create(GRect(0, 60, 144, 50));
	text_layer_set_background_color(s_time_layer, GColorClear);
	text_layer_set_text_color(s_time_layer, GColorWhite);
	text_layer_set_font(s_time_layer, s_time_font);
	text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));


	// DATE TEXT
	s_date_layer = text_layer_create(GRect(0, 144, 144, 26));
	text_layer_set_background_color(s_date_layer, GColorClear);
	text_layer_set_text_color(s_date_layer, GColorWhite);
	// Show values
	// static char rbuff[0x100];
	// r = layer_get_frame((Layer *)s_seconds_layer);
	// snprintf(rbuff, sizeof(rbuff), "%d,%d,%d", r.origin.y,r.size.w,r.size.h);
	// text_layer_set_text(s_date_layer,rbuff);
	text_layer_set_font(s_date_layer, s_date_font);
	text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));

	// TEMPERATURE TEXT
	s_temp_layer = text_layer_create(GRect(0, 100, 144, 26));
	text_layer_set_background_color(s_temp_layer, GColorClear);
	text_layer_set_text_color(s_temp_layer, GColorWhite);
	text_layer_set_font(s_temp_layer, s_charged_font);
	text_layer_set_text_alignment(s_temp_layer, GTextAlignmentCenter);
	//text_layer_set_text(s_temp_layer, "x");
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_temp_layer));
	
	
	// DAY TEXT
	s_day_layer = text_layer_create(GRect(0, 0, 144, 26));
	text_layer_set_background_color(s_day_layer, GColorClear);
	text_layer_set_text_color(s_day_layer, GColorWhite);
	text_layer_set_font(s_day_layer, s_day_font);
	text_layer_set_text_alignment(s_day_layer, GTextAlignmentCenter);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_day_layer));

	// Make sure the time is displayed from the start
	update_time();
	update_battery(battery_state_service_peek());
	update_bluetooth(bluetooth_connection_service_peek());
}

static void main_window_unload(Window *window) {
    // Destroy TextLayer
    text_layer_destroy(s_time_layer);
	text_layer_destroy(s_date_layer);
	text_layer_destroy(s_day_layer);
	text_layer_destroy(s_charged_layer);
	text_layer_destroy(s_temp_layer);
  
    // Unload GFont
    fonts_unload_custom_font(s_time_font);
	fonts_unload_custom_font(s_date_font);
	fonts_unload_custom_font(s_day_font);
	fonts_unload_custom_font(s_charged_font);
    
    // Destroy GBitmap and BitmapLayer
    gbitmap_destroy(s_battery_bitmap);
    bitmap_layer_destroy(s_battery_layer);
	gbitmap_destroy(s_bottom_bitmap);
    bitmap_layer_destroy(s_bottom_layer);
	gbitmap_destroy(s_charging_bitmap);
    bitmap_layer_destroy(s_charging_layer);
    
    // Destroy GBitmap and RotBitmapLayer
    gbitmap_destroy(s_seconds_bitmap);
    rot_bitmap_layer_destroy(s_seconds_layer);
	gbitmap_destroy(s_battery_mask_bitmap);
    rot_bitmap_layer_destroy(s_battery_mask_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	update_time();
	
	// Get weather update every 30 minutes
	  if(tick_time->tm_min % 2 == 0) {
	    // Begin dictionary
	    DictionaryIterator *iter;
	    app_message_outbox_begin(&iter);

	    // Add a key-value pair
	    dict_write_uint8(iter, 0, 0);

	    // Send the message!
	    app_message_outbox_send();
	  }
}

static void battery_state(BatteryChargeState charge){
	update_battery(charge);
}

static void bluetooth_connection_handler(bool connected){
	update_bluetooth(connected);
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Store incoming information
  static char temperature_buffer[8];
  static char conditions_buffer[32];
  static char weather_layer_buffer[32];
  
  // Read first item
  Tuple *t = dict_read_first(iterator);

  // For all items
  while(t != NULL) {
    // Which key was received?
    switch(t->key) {
    case KEY_TEMPERATURE:
      snprintf(temperature_buffer, sizeof(temperature_buffer), "%dC", (int)t->value->int32);
      break;
    case KEY_CONDITIONS:
      snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", t->value->cstring);
      break;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
      break;
    }

    // Look for next item
    t = dict_read_next(iterator);
  }
  
  // Assemble full string and display
  snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s", temperature_buffer);
  text_layer_set_text(s_temp_layer, weather_layer_buffer);
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
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

	// Register with BatteryStateService
	battery_state_service_subscribe(battery_state);
	
	// Register with BluetoothConnectionService
	bluetooth_connection_service_subscribe(bluetooth_connection_handler);
	
	// Register callbacks
	app_message_register_inbox_received(inbox_received_callback);
	app_message_register_inbox_dropped(inbox_dropped_callback);
	app_message_register_outbox_failed(outbox_failed_callback);
	app_message_register_outbox_sent(outbox_sent_callback);
	
	// Open AppMessage
	app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
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