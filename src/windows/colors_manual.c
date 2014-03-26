#include <pebble.h>
#include "colors_manual.h"
#include "../libs/pebble-assist.h"
#include "../common.h"
#include "lightlist.h"

#define MENU_NUM_SECTIONS 2

#define MENU_SECTION_STATUS 0
#define MENU_SECTION_MANUAL 1

#define MENU_SECTION_ROWS_MANUAL 3

#define MENU_ROW_MANUAL_HUE 0
#define MENU_ROW_MANUAL_SATURATION 1
#define MENU_ROW_MANUAL_BRIGHTNESS 2

static uint16_t menu_get_num_sections_callback(struct MenuLayer *menu_layer, void *callback_context);
static uint16_t menu_get_num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context);
static int16_t menu_get_header_height_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context);
static int16_t menu_get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);
static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *callback_context);
static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context);
static void menu_select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);
static void hue_select_callback(struct NumberWindow *number_window, void *context);
static void saturation_select_callback(struct NumberWindow *number_window, void *context);
static void brightness_select_callback(struct NumberWindow *number_window, void *context);

static Window *window;
static MenuLayer *menu_layer;
static NumberWindow *number_window[3];

enum {
	HUE,
	SATURATION,
	BRIGHTNESS,
};

void colors_manual_init(void) {
	window = window_create();

	menu_layer = menu_layer_create_fullscreen(window);
	menu_layer_set_callbacks(menu_layer, NULL, (MenuLayerCallbacks) {
		.get_num_sections = menu_get_num_sections_callback,
		.get_num_rows = menu_get_num_rows_callback,
		.get_header_height = menu_get_header_height_callback,
		.get_cell_height = menu_get_cell_height_callback,
		.draw_header = menu_draw_header_callback,
		.draw_row = menu_draw_row_callback,
		.select_click = menu_select_callback,
	});
	menu_layer_set_click_config_onto_window(menu_layer, window);
	menu_layer_add_to_window(menu_layer, window);

	number_window[HUE] = number_window_create("Hue", (NumberWindowCallbacks) { .selected = hue_select_callback }, NULL);
	number_window_set_min(number_window[HUE], 0);
	number_window_set_max(number_window[HUE], 100);
	number_window_set_step_size(number_window[HUE], 1);

	number_window[SATURATION] = number_window_create("Saturation", (NumberWindowCallbacks) { .selected = saturation_select_callback }, NULL);
	number_window_set_min(number_window[SATURATION], 0);
	number_window_set_max(number_window[SATURATION], 100);
	number_window_set_step_size(number_window[SATURATION], 1);

	number_window[BRIGHTNESS] = number_window_create("Brightness", (NumberWindowCallbacks) { .selected = brightness_select_callback }, NULL);
	number_window_set_min(number_window[BRIGHTNESS], 0);
	number_window_set_max(number_window[BRIGHTNESS], 100);
	number_window_set_step_size(number_window[BRIGHTNESS], 1);
}

void colors_manual_destroy(void) {
	for (int i = 0; i < 3; i++) {
		number_window_destroy(number_window[i]);
	}
	menu_layer_destroy_safe(menu_layer);
	window_destroy_safe(window);
}

void colors_manual_show(void) {
	window_stack_push(window, true);
}

void colors_manual_in_received_handler(DictionaryIterator *iter) {
	lightlist_in_received_handler(iter);
	menu_layer_reload_data_and_mark_dirty(menu_layer);
}

void colors_manual_out_sent_handler(DictionaryIterator *sent) {
}

void colors_manual_out_failed_handler(DictionaryIterator *failed, AppMessageResult reason) {
}

bool colors_manual_is_on_top() {
	return window == window_stack_get_top_window();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

static uint16_t menu_get_num_sections_callback(struct MenuLayer *menu_layer, void *callback_context) {
	return MENU_NUM_SECTIONS;
}

static uint16_t menu_get_num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
	switch (section_index) {
		case MENU_SECTION_STATUS:
			return 0;
		case MENU_SECTION_MANUAL:
			return MENU_SECTION_ROWS_MANUAL;
	}
	return 0;
}

static int16_t menu_get_header_height_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
	switch (section_index) {
		case MENU_SECTION_STATUS:
			return 28;
		case MENU_SECTION_MANUAL:
			return MENU_CELL_BASIC_HEADER_HEIGHT;
	}
	return 0;
}

static int16_t menu_get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	return 36;
}

static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *callback_context) {
	graphics_context_set_text_color(ctx, GColorBlack);
	switch (section_index) {
		case MENU_SECTION_STATUS:
			graphics_draw_text(ctx, light()->label, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), (GRect) { .origin = { 4, 2 }, .size = { 100, 22 } }, GTextOverflowModeFill, GTextAlignmentLeft, NULL);
			graphics_draw_text(ctx, light()->state, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), (GRect) { .origin = { 110, -3 }, .size = { 30, 26 } }, GTextOverflowModeFill, GTextAlignmentCenter, NULL);
			break;
		case MENU_SECTION_MANUAL:
			menu_cell_basic_header_draw(ctx, cell_layer, "Manual colors");
			break;
	}
}

static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
	char label[12] = "";
	switch (cell_index->section) {
		case MENU_SECTION_MANUAL:
			switch (cell_index->row) {
				case MENU_ROW_MANUAL_HUE:
					strcpy(label, "Hue");
					break;
				case MENU_ROW_MANUAL_SATURATION:
					strcpy(label, "Saturation");
					break;
				case MENU_ROW_MANUAL_BRIGHTNESS:
					strcpy(label, "Brightness");
					break;
			}
			break;
	}
	graphics_context_set_text_color(ctx, GColorBlack);
	graphics_draw_text(ctx, label, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), (GRect) { .origin = { 4, 0 }, .size = { PEBBLE_WIDTH - 8, 28 } }, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
}

static void menu_select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	switch (cell_index->section) {
		case MENU_SECTION_MANUAL:
			switch (cell_index->row) {
				case MENU_ROW_MANUAL_HUE:
					number_window_set_value(number_window[HUE], light()->color.hue);
					window_stack_push((Window*)number_window[HUE], true);
					break;
				case MENU_ROW_MANUAL_SATURATION:
					number_window_set_value(number_window[SATURATION], light()->color.saturation);
					window_stack_push((Window*)number_window[SATURATION], true);
					break;
				case MENU_ROW_MANUAL_BRIGHTNESS:
					number_window_set_value(number_window[BRIGHTNESS], light()->color.brightness);
					window_stack_push((Window*)number_window[BRIGHTNESS], true);
					break;
			}
			break;
	}
}

static void hue_select_callback(struct NumberWindow *number_window, void *context) {
	light()->color.hue = number_window_get_value(number_window);
	light_update_color();
	window_stack_pop(true);
}

static void saturation_select_callback(struct NumberWindow *number_window, void *context) {
	light()->color.saturation = number_window_get_value(number_window);
	light_update_color();
	window_stack_pop(true);
}

static void brightness_select_callback(struct NumberWindow *number_window, void *context) {
	light()->color.brightness = number_window_get_value(number_window);
	light_update_color();
	window_stack_pop(true);
}
