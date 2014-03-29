#include <pebble.h>
#include "colors_dim.h"
#include "../libs/pebble-assist.h"
#include "../common.h"
#include "lightlist.h"

#define MENU_NUM_SECTIONS 2

#define MENU_SECTION_STATUS 0
#define MENU_SECTION_PRESETS 1

#define MENU_SECTION_ROWS_PRESETS 6

#define MENU_ROW_PRESET_100 0
#define MENU_ROW_PRESET_80 1
#define MENU_ROW_PRESET_60 2
#define MENU_ROW_PRESET_40 3
#define MENU_ROW_PRESET_20 4
#define MENU_ROW_PRESET_1 5

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

void colors_dim_init(void) {
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

void colors_dim_destroy(void) {
	menu_layer_destroy_safe(menu_layer);
	window_destroy_safe(window);
}

void colors_dim_show(void) {
	window_stack_push(window, true);
}

void colors_dim_reload_data_and_mark_dirty(void) {
	menu_layer_reload_data_and_mark_dirty(menu_layer);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

static uint16_t menu_get_num_sections_callback(struct MenuLayer *menu_layer, void *callback_context) {
	return MENU_NUM_SECTIONS;
}

static uint16_t menu_get_num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
	switch (section_index) {
		case MENU_SECTION_STATUS:
			return 0;
		case MENU_SECTION_PRESETS:
			return MENU_SECTION_ROWS_PRESETS;
	}
	return 0;
}

static int16_t menu_get_header_height_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
	switch (section_index) {
		case MENU_SECTION_STATUS:
			return 28;
		case MENU_SECTION_PRESETS:
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
		case MENU_SECTION_PRESETS:
			menu_cell_basic_header_draw(ctx, cell_layer, "Dim Presets");
			break;
	}
}

static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
	char label[4] = "";
	switch (cell_index->section) {
		case MENU_SECTION_PRESETS:
			switch (cell_index->row) {
				case MENU_ROW_PRESET_100:
					strcpy(label, "100%");
					break;
				case MENU_ROW_PRESET_80:
					strcpy(label, "80%");
					break;
				case MENU_ROW_PRESET_60:
					strcpy(label, "60%");
					break;
				case MENU_ROW_PRESET_40:
					strcpy(label, "40%");
					break;
				case MENU_ROW_PRESET_20:
					strcpy(label, "20%");
					break;
				case MENU_ROW_PRESET_1:
					strcpy(label, "1%");
					break;
			}
			break;
	}
	graphics_context_set_text_color(ctx, GColorBlack);
	graphics_draw_text(ctx, label, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), (GRect) { .origin = { 4, 0 }, .size = { PEBBLE_WIDTH - 8, 28 } }, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
}

static void menu_select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	switch (cell_index->section) {
		case MENU_SECTION_PRESETS:
			switch (cell_index->row) {
				case MENU_ROW_PRESET_100:
					light()->color = (Color) { light()->color.hue, light()->color.saturation, 100 };
					light_update_color();
					break;
				case MENU_ROW_PRESET_80:
					light()->color = (Color) { light()->color.hue, light()->color.saturation, 80 };
					light_update_color();
					break;
				case MENU_ROW_PRESET_60:
					light()->color = (Color) { light()->color.hue, light()->color.saturation, 60 };
					light_update_color();
					break;
				case MENU_ROW_PRESET_40:
					light()->color = (Color) { light()->color.hue, light()->color.saturation, 40 };
					light_update_color();
					break;
				case MENU_ROW_PRESET_20:
					light()->color = (Color) { light()->color.hue, light()->color.saturation, 20 };
					light_update_color();
					break;
				case MENU_ROW_PRESET_1:
					light()->color = (Color) { light()->color.hue, light()->color.saturation, 1 };
					light_update_color();
					break;
			}
			break;
	}
}

static void menu_select_long_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
}
