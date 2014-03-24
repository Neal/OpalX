#include <pebble.h>
#include "colors_manual.h"
#include "../libs/pebble-assist.h"
#include "../common.h"
#include "../progress_bar.h"
#include "lights.h"

#define NUM_TYPES 3

static void set_colors();
static void back_single_click_handler(ClickRecognizerRef recognizer, void *context);
static void up_single_click_handler(ClickRecognizerRef recognizer, void *context);
static void down_single_click_handler(ClickRecognizerRef recognizer, void *context);
static void select_single_click_handler(ClickRecognizerRef recognizer, void *context);
static void select_long_click_handler(ClickRecognizerRef recognizer, void *context);
static void click_config_provider(void *context);
static void window_load(Window *window);
static void window_unload(Window *window);

static Window *window;

static ActionBarLayer *action_bar;

static GBitmap *action_icon_up;
static GBitmap *action_icon_down;
static GBitmap *action_icon_select;

static char *title_text[NUM_TYPES] = { "Hue", "Saturation", "Brightness" };

static TextLayer *title_text_layer[NUM_TYPES];

static ProgressBarLayer *progress_bar[NUM_TYPES];

static InverterLayer *inverter_layer;

typedef enum {
	HUE,
	SATURATION,
	BRIGHTNESS,
} CONTROLLING;

static CONTROLLING controlling;

void colors_manual_init(void) {
	window = window_create();
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});
	window_stack_push(window, true);
}

void colors_manual_destroy(void) {
	window_destroy_safe(window);
}

void colors_manual_in_received_handler(DictionaryIterator *iter) {
}

void colors_manual_out_sent_handler(DictionaryIterator *sent) {
}

void colors_manual_out_failed_handler(DictionaryIterator *failed, AppMessageResult reason) {
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

static void set_colors() {
}

static void update_display() {
	inverter_layer_destroy(inverter_layer);
	switch (controlling) {
		case HUE:
			inverter_layer = inverter_layer_create((GRect) { .origin = { 0, 5 }, .size = { PEBBLE_WIDTH - 20, 40 } });
			break;
		case SATURATION:
			inverter_layer = inverter_layer_create((GRect) { .origin = { 0, 56 }, .size = { PEBBLE_WIDTH - 20, 40 } });
			break;
		case BRIGHTNESS:
			inverter_layer = inverter_layer_create((GRect) { .origin = { 0, 107 }, .size = { PEBBLE_WIDTH - 20, 40 } });
			break;
	}
	layer_add_child(window_get_root_layer(window), inverter_layer_get_layer(inverter_layer));
}

static void back_single_click_handler(ClickRecognizerRef recognizer, void *context) {
	colors_manual_destroy();
	window_stack_pop(true);
}

static void up_single_click_handler(ClickRecognizerRef recognizer, void *context) {
}

static void down_single_click_handler(ClickRecognizerRef recognizer, void *context) {
}

static void select_single_click_handler(ClickRecognizerRef recognizer, void *context) {
}

static void select_long_click_handler(ClickRecognizerRef recognizer, void *context) {
	controlling = (controlling + 1) % NUM_TYPES;
	update_display();
}

static void click_config_provider(void *context) {
	window_single_click_subscribe(BUTTON_ID_BACK, back_single_click_handler);
	window_single_repeating_click_subscribe(BUTTON_ID_UP, 500, up_single_click_handler);
	window_single_repeating_click_subscribe(BUTTON_ID_DOWN, 500, down_single_click_handler);
	window_single_click_subscribe(BUTTON_ID_SELECT, select_single_click_handler);
	window_long_click_subscribe(BUTTON_ID_SELECT, 500, select_long_click_handler, NULL);
}

static void window_load(Window *window) {
	action_icon_up = gbitmap_create_with_resource(RESOURCE_ID_ICON_UP);
	action_icon_down = gbitmap_create_with_resource(RESOURCE_ID_ICON_DOWN);
	action_icon_select = gbitmap_create_with_resource(RESOURCE_ID_ICON_SELECT);

	action_bar = action_bar_layer_create();
	action_bar_layer_add_to_window(action_bar, window);
	action_bar_layer_set_click_config_provider(action_bar, click_config_provider);

	action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, action_icon_up);
	action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, action_icon_down);
	action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, action_icon_select);

	for (int i = 0; i < NUM_TYPES; i++) {
		title_text_layer[i] = text_layer_create((GRect) { .origin = { 4, 52 * i }, .size = { PEBBLE_WIDTH - 28, 28 } });
		text_layer_set_text(title_text_layer[i], title_text[i]);
		text_layer_set_text_color(title_text_layer[i], GColorBlack);
		text_layer_set_background_color(title_text_layer[i], GColorClear);
		text_layer_set_font(title_text_layer[i], fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
		layer_add_child(window_get_root_layer(window), text_layer_get_layer(title_text_layer[i]));

		progress_bar[i] = progress_bar_layer_create((GRect) { .origin = { 4, (52 * i) + 32 }, .size = { PEBBLE_WIDTH - 28, 8 } });
		progress_bar_layer_set_orientation(progress_bar[i], ProgressBarOrientationHorizontal);
		progress_bar_layer_set_range(progress_bar[i], 0, 100);
		progress_bar_layer_set_frame_color(progress_bar[i], GColorBlack);
		progress_bar_layer_set_bar_color(progress_bar[i], GColorBlack);
		layer_add_child(window_get_root_layer(window), progress_bar[i]);
	}

	update_display();
}

static void window_unload(Window *window) {
	gbitmap_destroy(action_icon_up);
	gbitmap_destroy(action_icon_down);
	gbitmap_destroy(action_icon_select);
	action_bar_layer_destroy(action_bar);
	inverter_layer_destroy(inverter_layer);
	for (int i = 0; i < NUM_TYPES; i++) {
		text_layer_destroy(title_text_layer[i]);
		progress_bar_layer_destroy(progress_bar[i]);
	}
}
