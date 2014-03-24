#include <pebble.h>
#include "colors_default.h"
#include "../libs/pebble-assist.h"
#include "../common.h"
#include "lights.h"

#define MENU_NUM_SECTIONS 2

#define MENU_SECTION_STATUS 0
#define MENU_SECTION_DEFAULT 1

#define MENU_SECTION_ROWS_DEFAULT 6

#define MENU_ROW_DEFAULT_WHITE 0
#define MENU_ROW_DEFAULT_RED 1
#define MENU_ROW_DEFAULT_ORANGE 2
#define MENU_ROW_DEFAULT_YELLOW 3
#define MENU_ROW_DEFAULT_GREEN 4
#define MENU_ROW_DEFAULT_BLUE 5

static void refresh();
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

static bool out_failed = false;
static bool conn_timeout = false;
static bool conn_error = false;
static bool server_error = false;

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

	window_stack_push(window, true);
}

void colors_default_destroy(void) {
	menu_layer_destroy_safe(menu_layer);
	window_destroy_safe(window);
}

void colors_default_in_received_handler(DictionaryIterator *iter) {
	Tuple *index_tuple = dict_find(iter, KEY_INDEX);
	Tuple *label_tuple = dict_find(iter, KEY_LABEL);
	Tuple *color_tuple = dict_find(iter, KEY_COLOR);
	Tuple *state_tuple = dict_find(iter, KEY_STATE);
	Tuple *error_tuple = dict_find(iter, KEY_ERROR);

	if (error_tuple) {
		if (strcmp(error_tuple->value->cstring, "timeout") != 0) {
			conn_timeout = true;
		} else if (strcmp(error_tuple->value->cstring, "error") != 0) {
			conn_error = true;
		} else if (strcmp(error_tuple->value->cstring, "server_error") != 0) {
			server_error = true;
		}
		menu_layer_reload_data_and_mark_dirty(menu_layer);
	}
	else if (index_tuple && label_tuple && color_tuple && state_tuple) {
		out_failed = false;
		conn_timeout = false;
		conn_error = false;
		server_error = false;
		if (index_tuple->value->int16 == light()->index) {
			strncpy(light()->label, label_tuple->value->cstring, sizeof(light()->label) - 1);
			strncpy(light()->color, color_tuple->value->cstring, sizeof(light()->color) - 1);
			strncpy(light()->state, state_tuple->value->cstring, sizeof(light()->state) - 1);
		}
		menu_layer_reload_data_and_mark_dirty(menu_layer);
	}
}

void colors_default_out_sent_handler(DictionaryIterator *sent) {
}

void colors_default_out_failed_handler(DictionaryIterator *failed, AppMessageResult reason) {
	out_failed = true;
	menu_layer_reload_data_and_mark_dirty(menu_layer);
}

bool colors_default_is_on_top() {
	return window == window_stack_get_top_window();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

static void refresh() {
	out_failed = false;
	conn_timeout = false;
	conn_error = false;
	server_error = false;
	menu_layer_set_selected_index(menu_layer, (MenuIndex) { .row = 0, .section = 0 }, MenuRowAlignBottom, false);
	menu_layer_reload_data_and_mark_dirty(menu_layer);
	app_message_outbox_send();
}

static uint16_t menu_get_num_sections_callback(struct MenuLayer *menu_layer, void *callback_context) {
	return MENU_NUM_SECTIONS;
}

static uint16_t menu_get_num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
	switch (section_index) {
		case MENU_SECTION_STATUS:
			return 0;
		case MENU_SECTION_DEFAULT:
			return MENU_SECTION_ROWS_DEFAULT;
	}
	return 0;
}

static int16_t menu_get_header_height_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
	switch (section_index) {
		case MENU_SECTION_STATUS:
			return 28;
		case MENU_SECTION_DEFAULT:
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
			if (out_failed) {
				graphics_draw_text(ctx, "Phone unreachable!", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), (GRect) { .origin = { 4, 0 }, .size = { PEBBLE_WIDTH - 8, 22 } }, GTextOverflowModeFill, GTextAlignmentLeft, NULL);
			} else if (conn_timeout) {
				graphics_draw_text(ctx, "Connection timed out!", fonts_get_system_font(FONT_KEY_GOTHIC_18), (GRect) { .origin = { 4, 0 }, .size = { PEBBLE_WIDTH - 8, 44 } }, GTextOverflowModeFill, GTextAlignmentLeft, NULL);
			} else if (conn_error) {
				graphics_draw_text(ctx, "HTTP Error!", fonts_get_system_font(FONT_KEY_GOTHIC_18), (GRect) { .origin = { 4, 0 }, .size = { PEBBLE_WIDTH - 8, 44 } }, GTextOverflowModeFill, GTextAlignmentLeft, NULL);
			} else if (server_error) {
				graphics_draw_text(ctx, "Server error!", fonts_get_system_font(FONT_KEY_GOTHIC_18), (GRect) { .origin = { 4, 0 }, .size = { PEBBLE_WIDTH - 8, 44 } }, GTextOverflowModeFill, GTextAlignmentLeft, NULL);
			} else {
				graphics_draw_text(ctx, light()->label, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), (GRect) { .origin = { 4, 2 }, .size = { 100, 22 } }, GTextOverflowModeFill, GTextAlignmentLeft, NULL);
				graphics_draw_text(ctx, light()->state, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), (GRect) { .origin = { 110, -3 }, .size = { 30, 26 } }, GTextOverflowModeFill, GTextAlignmentCenter, NULL);
			}
			break;
		case MENU_SECTION_DEFAULT:
			menu_cell_basic_header_draw(ctx, cell_layer, "Default colors");
			break;
	}
}

static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
	char label[13] = "";
	switch (cell_index->section) {
		case MENU_SECTION_DEFAULT:
			switch (cell_index->row) {
				case MENU_ROW_DEFAULT_WHITE:
					strcpy(label, "White");
					break;
				case MENU_ROW_DEFAULT_RED:
					strcpy(label, "Red");
					break;
				case MENU_ROW_DEFAULT_ORANGE:
					strcpy(label, "Orange");
					break;
				case MENU_ROW_DEFAULT_YELLOW:
					strcpy(label, "Yellow");
					break;
				case MENU_ROW_DEFAULT_GREEN:
					strcpy(label, "Green");
					break;
				case MENU_ROW_DEFAULT_BLUE:
					strcpy(label, "Blue");
					break;
			}
			break;
	}
	graphics_context_set_text_color(ctx, GColorBlack);
	graphics_draw_text(ctx, label, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), (GRect) { .origin = { 4, 0 }, .size = { PEBBLE_WIDTH - 8, 28 } }, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
}

static void menu_select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	switch (cell_index->section) {
		case MENU_SECTION_DEFAULT:
			switch (cell_index->row) {
				case MENU_ROW_DEFAULT_WHITE:
					break;
				case MENU_ROW_DEFAULT_RED:
					break;
				case MENU_ROW_DEFAULT_ORANGE:
					break;
				case MENU_ROW_DEFAULT_YELLOW:
					break;
				case MENU_ROW_DEFAULT_GREEN:
					break;
				case MENU_ROW_DEFAULT_BLUE:
					break;
			}
			break;
	}
}

static void menu_select_long_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	refresh();
}
