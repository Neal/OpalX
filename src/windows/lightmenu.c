#include <pebble.h>
#include "lightmenu.h"
#include "../libs/pebble-assist.h"
#include "../common.h"
#include "lightlist.h"
#include "colors_custom.h"
#include "colors_default.h"
#include "colors_manual.h"

#define MENU_NUM_SECTIONS 3

#define MENU_SECTION_STATUS 0
#define MENU_SECTION_TOGGLE 1
#define MENU_SECTION_COLORS 2

#define MENU_SECTION_ROWS_STATUS 0
#define MENU_SECTION_ROWS_TOGGLE 1
#define MENU_SECTION_ROWS_COLORS 2

#define MENU_ROW_TOGGLE 0

#define MENU_ROW_COLORS_CUSTOM 99
#define MENU_ROW_COLORS_DEFAULT 0
#define MENU_ROW_COLORS_MANUAL 1

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

void lightmenu_init(void) {
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

	colors_custom_init();
	colors_default_init();
	colors_manual_init();
}

void lightmenu_destroy(void) {
	colors_custom_destroy();
	colors_default_destroy();
	colors_manual_destroy();
	menu_layer_destroy_safe(menu_layer);
	window_destroy_safe(window);
}

void lightmenu_show(void) {
	window_stack_push(window, true);
}

void lightmenu_reload_data_and_mark_dirty(void) {
	menu_layer_reload_data_and_mark_dirty(menu_layer);
	colors_default_reload_data_and_mark_dirty();
	colors_manual_reload_data_and_mark_dirty();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

static uint16_t menu_get_num_sections_callback(struct MenuLayer *menu_layer, void *callback_context) {
	return MENU_NUM_SECTIONS;
}

static uint16_t menu_get_num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
	switch (section_index) {
		case MENU_SECTION_STATUS:
			return 0;
		case MENU_SECTION_TOGGLE:
			return MENU_SECTION_ROWS_TOGGLE;
		case MENU_SECTION_COLORS:
			return MENU_SECTION_ROWS_COLORS;
	}
	return 0;
}

static int16_t menu_get_header_height_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
	switch (section_index) {
		case MENU_SECTION_STATUS:
			return 28;
		case MENU_SECTION_TOGGLE:
			return 0;
		case MENU_SECTION_COLORS:
			return 18;
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
			graphics_draw_text(ctx, "Colors", fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), (GRect) { .origin = { 4, 0 }, .size = { 60, 18 } }, GTextOverflowModeFill, GTextAlignmentLeft, NULL);
			break;
	}
}

static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
	char label[13] = "";
	switch (cell_index->section) {
		case MENU_SECTION_TOGGLE:
			switch (cell_index->row) {
				case MENU_ROW_TOGGLE:
					strcpy(label, "Toggle");
					break;
			}
			break;
		case MENU_SECTION_COLORS:
			switch (cell_index->row) {
				case MENU_ROW_COLORS_CUSTOM:
					strcpy(label, "Custom");
					break;
				case MENU_ROW_COLORS_DEFAULT:
					strcpy(label, "Default");
					break;
				case MENU_ROW_COLORS_MANUAL:
					strcpy(label, "Manual");
					break;
			}
			break;
	}
	graphics_context_set_text_color(ctx, GColorBlack);
	graphics_draw_text(ctx, label, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), (GRect) { .origin = { 4, 0 }, .size = { PEBBLE_WIDTH - 8, 28 } }, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
}

static void menu_select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	switch (cell_index->section) {
		case MENU_SECTION_TOGGLE:
			switch (cell_index->row) {
				case MENU_ROW_TOGGLE:
					light_toggle();
					break;
			}
			break;
		case MENU_SECTION_COLORS:
			switch (cell_index->row) {
				case MENU_ROW_COLORS_CUSTOM:
					colors_custom_show();
					break;
				case MENU_ROW_COLORS_DEFAULT:
					colors_default_show();
					break;
				case MENU_ROW_COLORS_MANUAL:
					colors_manual_show();
					break;
			}
			break;
	}
}

static void menu_select_long_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
}
