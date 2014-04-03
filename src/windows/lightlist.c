#include <pebble.h>
#include "lightlist.h"
#include "../libs/pebble-assist.h"
#include "../common.h"
#include "../light.h"
#include "../settings.h"
#include "settings.h"
#include "lightmenu.h"

#define MENU_NUM_SECTIONS 4

#define MENU_SECTION_ALL 0
#define MENU_SECTION_OTHER 3

#define MENU_SECTION_ROWS_ALL 1
#define MENU_SECTION_ROWS_OTHER 1

#define MENU_ROW_OTHER_SETTINGS 0

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

void lightlist_init(void) {
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

	settings_init();
	lightmenu_init();
}

void lightlist_deinit(void) {
	settings_deinit();
	lightmenu_deinit();
	menu_layer_destroy_safe(menu_layer);
	window_destroy_safe(window);
}

void lightlist_reload_data_and_mark_dirty(void) {
	menu_layer_reload_data_and_mark_dirty(menu_layer);
	lightmenu_reload_data_and_mark_dirty();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

static uint16_t menu_get_num_sections_callback(struct MenuLayer *menu_layer, void *callback_context) {
	return MENU_NUM_SECTIONS;
}

static uint16_t menu_get_num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
	if (section_index == MENU_SECTION_ALL) {
		return MENU_SECTION_ROWS_ALL;
	} else if (section_index == menu_section_lights) {
		return num_lights;
	} else if (section_index == menu_section_tags) {
		return num_tags;
	} else if (section_index == MENU_SECTION_OTHER) {
		return MENU_SECTION_ROWS_OTHER;
	} else {
		return 0;
	}
}

static int16_t menu_get_header_height_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
	if (section_index == menu_section_lights) {
		return num_lights ? MENU_CELL_BASIC_HEADER_HEIGHT : 1;
	} else if (section_index == menu_section_tags) {
		return num_tags ? MENU_CELL_BASIC_HEADER_HEIGHT : 1;
	} else if (section_index == MENU_SECTION_OTHER) {
		return MENU_CELL_BASIC_HEADER_HEIGHT;
	}
	return 1;
}

static int16_t menu_get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	if (cell_index->section == MENU_SECTION_ALL) {
		if (error) {
			return graphics_text_layout_get_content_size(error, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), (GRect) { .origin = { 4, 2 }, .size = { PEBBLE_WIDTH - 8, 88 } }, GTextOverflowModeFill, GTextAlignmentLeft).h + 10;
		}
		return 30;
	} else if (cell_index->section == menu_section_lights) {
		return 30;
	} else if (cell_index->section == menu_section_tags) {
		return 30;
	} else if (cell_index->section == MENU_SECTION_OTHER) {
		return 30;
	} else {
		return 0;
	}
}

static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *callback_context) {
	if (section_index == menu_section_lights) {
		menu_cell_basic_header_draw(ctx, cell_layer, "Lights");
	} else if (section_index == menu_section_tags) {
		menu_cell_basic_header_draw(ctx, cell_layer, "Tags");
	} else if (section_index == MENU_SECTION_OTHER) {
		menu_cell_basic_header_draw(ctx, cell_layer, "Other");
	}
}

static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
	graphics_context_set_text_color(ctx, GColorBlack);
	if (cell_index->section == MENU_SECTION_ALL) {
		if (error) {
			graphics_draw_text(ctx, error, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), (GRect) { .origin = { 4, 2 }, .size = { PEBBLE_WIDTH - 8, 88 } }, GTextOverflowModeFill, GTextAlignmentLeft, NULL);
		} else if (num_lights == 0) {
			graphics_draw_text(ctx, "Loading lights...", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), (GRect) { .origin = { 4, 2 }, .size = { PEBBLE_WIDTH - 8, 22 } }, GTextOverflowModeFill, GTextAlignmentLeft, NULL);
		} else {
			graphics_draw_text(ctx, all_lights->label, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), (GRect) { .origin = { 4, 2 }, .size = { PEBBLE_WIDTH - 8, 22 } }, GTextOverflowModeFill, GTextAlignmentLeft, NULL);
		}
	} else if (cell_index->section == menu_section_lights) {
		graphics_draw_text(ctx, lights[cell_index->row].label, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), (GRect) { .origin = { 4, 2 }, .size = { 100, 22 } }, GTextOverflowModeFill, GTextAlignmentLeft, NULL);
		graphics_draw_text(ctx, lights[cell_index->row].state, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), (GRect) { .origin = { 110, -3 }, .size = { 30, 26 } }, GTextOverflowModeFill, GTextAlignmentCenter, NULL);
	} else if (cell_index->section == menu_section_tags) {
		graphics_draw_text(ctx, tags[cell_index->row].label, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), (GRect) { .origin = { 4, 2 }, .size = { 100, 22 } }, GTextOverflowModeFill, GTextAlignmentLeft, NULL);
		graphics_draw_text(ctx, tags[cell_index->row].state, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), (GRect) { .origin = { 110, -3 }, .size = { 30, 26 } }, GTextOverflowModeFill, GTextAlignmentCenter, NULL);
	} else if (cell_index->section == MENU_SECTION_OTHER) {
		graphics_draw_text(ctx, "Settings", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), (GRect) { .origin = { 4, 2 }, .size = { PEBBLE_WIDTH - 8, 22 } }, GTextOverflowModeFill, GTextAlignmentLeft, NULL);
	}
}

static void menu_select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	if (cell_index->section == MENU_SECTION_ALL) {
		if (num_lights == 0) return;
		selected_index = cell_index->row;
		selected_type = KEY_TYPE_ALL;
		lightmenu_show();
	} else if (cell_index->section == menu_section_lights) {
		selected_index = cell_index->row;
		selected_type = KEY_TYPE_LIGHT;
		lightmenu_show();
	} else if (cell_index->section == menu_section_tags) {
		selected_index = cell_index->row;
		selected_type = KEY_TYPE_TAG;
		lightmenu_show();
	} else if (cell_index->section == MENU_SECTION_OTHER) {
		settings_show();
	}
}

static void menu_select_long_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	if (cell_index->section == MENU_SECTION_ALL) {
		if (num_lights == 0) {
			app_message_outbox_send();
			return;
		}
		selected_index = cell_index->row;
		selected_type = KEY_TYPE_ALL;
		light_toggle();
	} else if (cell_index->section == menu_section_lights) {
		selected_index = cell_index->row;
		selected_type = KEY_TYPE_LIGHT;
		light_toggle();
	} else if (cell_index->section == menu_section_tags) {
		selected_index = cell_index->row;
		selected_type = KEY_TYPE_TAG;
		light_toggle();
	}
}
