#include <pebble.h>
#include "colors_default.h"
#include "../libs/pebble-assist.h"
#include "../common.h"
#include "lights.h"

#define MENU_NUM_SECTIONS 2

#define MENU_SECTION_STATUS 0
#define MENU_SECTION_COLORS 1

#define MENU_SECTION_ROWS_COLORS 9

#define MENU_ROW_COLORS_WHITE 0
#define MENU_ROW_COLORS_RED 1
#define MENU_ROW_COLORS_ORANGE 2
#define MENU_ROW_COLORS_YELLOW 3
#define MENU_ROW_COLORS_GREEN 4
#define MENU_ROW_COLORS_TEAL 5
#define MENU_ROW_COLORS_BLUE 6
#define MENU_ROW_COLORS_PURPLE 7
#define MENU_ROW_COLORS_PINK 8

static uint16_t menu_get_num_sections_callback(struct MenuLayer *menu_layer, void *callback_context);
static uint16_t menu_get_num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context);
static int16_t menu_get_header_height_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context);
static int16_t menu_get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);
static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *callback_context);
static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context);
static void menu_select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);
static void menu_select_long_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);

static Window *window;
static MenuLayer *menu_layer;

void colors_default_init(void) {
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
		.select_long_click = menu_select_long_callback,
	});
	menu_layer_set_click_config_onto_window(menu_layer, window);
	menu_layer_add_to_window(menu_layer, window);
}

void colors_default_destroy(void) {
	menu_layer_destroy_safe(menu_layer);
	window_destroy_safe(window);
}

void colors_default_show(void) {
	window_stack_push(window, true);
}

void colors_default_in_received_handler(DictionaryIterator *iter) {
	lights_in_received_handler(iter);
	menu_layer_reload_data_and_mark_dirty(menu_layer);
}

void colors_default_out_sent_handler(DictionaryIterator *sent) {
}

void colors_default_out_failed_handler(DictionaryIterator *failed, AppMessageResult reason) {
}

bool colors_default_is_on_top() {
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
		case MENU_SECTION_COLORS:
			return MENU_SECTION_ROWS_COLORS;
	}
	return 0;
}

static int16_t menu_get_header_height_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
	switch (section_index) {
		case MENU_SECTION_STATUS:
			return 28;
		case MENU_SECTION_COLORS:
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
		case MENU_SECTION_COLORS:
			menu_cell_basic_header_draw(ctx, cell_layer, "Default colors");
			break;
	}
}

static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
	char label[13] = "";
	switch (cell_index->section) {
		case MENU_SECTION_COLORS:
			switch (cell_index->row) {
				case MENU_ROW_COLORS_WHITE:
					strcpy(label, "White");
					break;
				case MENU_ROW_COLORS_RED:
					strcpy(label, "Red");
					break;
				case MENU_ROW_COLORS_ORANGE:
					strcpy(label, "Orange");
					break;
				case MENU_ROW_COLORS_YELLOW:
					strcpy(label, "Yellow");
					break;
				case MENU_ROW_COLORS_GREEN:
					strcpy(label, "Green");
					break;
				case MENU_ROW_COLORS_TEAL:
					strcpy(label, "Teal");
					break;
				case MENU_ROW_COLORS_BLUE:
					strcpy(label, "Blue");
					break;
				case MENU_ROW_COLORS_PURPLE:
					strcpy(label, "Purple");
					break;
				case MENU_ROW_COLORS_PINK:
					strcpy(label, "Pink");
					break;
			}
			break;
	}
	graphics_context_set_text_color(ctx, GColorBlack);
	graphics_draw_text(ctx, label, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), (GRect) { .origin = { 4, 0 }, .size = { PEBBLE_WIDTH - 8, 28 } }, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
}

static void menu_select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	switch (cell_index->section) {
		case MENU_SECTION_COLORS:
			switch (cell_index->row) {
				case MENU_ROW_COLORS_WHITE:
					light()->color = (Color) { 0, 0, light()->color.brightness };
					light_update_color();
					break;
				case MENU_ROW_COLORS_RED:
					light()->color = (Color) { 0, 100, light()->color.brightness };
					light_update_color();
					break;
				case MENU_ROW_COLORS_ORANGE:
					light()->color = (Color) { 10, 100, light()->color.brightness };
					light_update_color();
					break;
				case MENU_ROW_COLORS_YELLOW:
					light()->color = (Color) { 15, 100, light()->color.brightness };
					light_update_color();
					break;
				case MENU_ROW_COLORS_GREEN:
					light()->color = (Color) { 30, 100, light()->color.brightness };
					light_update_color();
					break;
				case MENU_ROW_COLORS_TEAL:
					light()->color = (Color) { 50, 100, light()->color.brightness };
					light_update_color();
					break;
				case MENU_ROW_COLORS_BLUE:
					light()->color = (Color) { 65, 100, light()->color.brightness };
					light_update_color();
					break;
				case MENU_ROW_COLORS_PURPLE:
					light()->color = (Color) { 80, 100, light()->color.brightness };
					light_update_color();
					break;
				case MENU_ROW_COLORS_PINK:
					light()->color = (Color) { 90, 100, light()->color.brightness };
					light_update_color();
					break;
			}
			break;
	}
}

static void menu_select_long_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
}
