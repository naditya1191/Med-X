#include <pebble.h>
static Window *s_main_window;
static TextLayer *s_text_layer;

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	
	// Get weather update every 30 minutes
	  if(tick_time->tm_sec % 1 == 0) {
	    // Begin dictionary
	    DictionaryIterator *iter;
	    app_message_outbox_begin(&iter);

	    // Add a key-value pair
	    dict_write_uint8(iter, 0, 0);

	    // Send the message!
	    app_message_outbox_send();
	  }
}

static void main_window_load(Window *window) {
  	
	// CHARGED
	s_text_layer = text_layer_create(GRect(0, 40, 144, 168));
	text_layer_set_background_color(s_text_layer, GColorBlack);
	text_layer_set_text_color(s_text_layer, GColorWhite);
	text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);
	//text_layer_set_text(s_charged_layer, "");
	text_layer_set_font(s_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_text_layer));
	//layer_set_hidden(text_layer_get_layer(s_charged_layer), true);

	// Make sure the time is displayed from the start
	//update_data();
}

static void main_window_unload(Window *window) {
    // Destroy TextLayer
    text_layer_destroy(s_text_layer);
    
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Store incoming information
  static char body_counter_buffer[32];

  // Read first item
  Tuple *t = dict_read_first(iterator);

  // For all items
  while(t != NULL) {
    // Which key was received?
    switch(t->key) {
    case 0:
		if(t->value->int32 == 1){
			snprintf(body_counter_buffer, sizeof(body_counter_buffer), "%s", "ALERT!! Bad Back Posture Detected!");
			vibes_double_pulse();
		}else if(t->value->int32 == 2){
			snprintf(body_counter_buffer, sizeof(body_counter_buffer), "%s", "ALERT!! Bad Neck Posture Detected!");
			vibes_double_pulse();
		}else if(t->value->int32 == 3){
			snprintf(body_counter_buffer, sizeof(body_counter_buffer), "%s", "ALERT!! Bad Neck and Back Posture Detected!");
			vibes_double_pulse();
		}else{
      		snprintf(body_counter_buffer, sizeof(body_counter_buffer), "%s", "Keep it up. Good Posture! :)");
		}
      break;
	
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
      break;
    }

    // Look for next item
    t = dict_read_next(iterator);
  }

  // Assemble full string and display
  //snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s", temperature_buffer);
  text_layer_set_text(s_text_layer, body_counter_buffer);
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