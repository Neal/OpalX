#include <pebble.h>
#include "lightlist.h"
#include "../libs/pebble-assist.h"
#include "../common.h"
#include "../settings.h"
#include "settings.h"
#include "lightmenu.h"

#define MENU_NUM_SECTIONS 4

#define MENU_SECTION_ALL 0
#define MENU_SECTION_OTHER 3

#define MENU_SECTION_ROWS_ALL 1
#define MENU_SECTION_ROWS_OTHER 1

#define MENU_ROW_OTHER_SETTINGS 0

static Light* all_lights;
static Light* lights;
static Light* tags;

static void all_menu_layer_reload_data_and_mark_dirty();
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

static char* error;

static uint8_t num_lights = 0;
static uint8_t num_tags = 0;
static uint8_t selected_index;
static uint8_t selected_type;
static uint8_t menu_section_lights = 1;
static uint8_t menu_section_tags = 2;

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

	all_lights = malloc(sizeof(Light));
	all_lights->index = 0;
	strncpy(all_lights->label, "All Lights", sizeof(all_lights->label) - 1);
	all_lights->color = (Color) {
		.hue = 50,
		.saturation = 100,
		.brightness = 100,
	};

	lightlist_update_settings();

	settings_init();
	lightmenu_init();
}

void lightlist_destroy(void) {
	if (error) free(error);
	if (all_lights) free(all_lights);
	if (lights) free(lights);
	if (tags) free(tags);
	settings_destroy();
	lightmenu_destroy();
	menu_layer_destroy_safe(menu_layer);
	window_destroy_safe(window);
}

void lightlist_in_received_handler(DictionaryIterator *iter) {
	if (!dict_find(iter, KEY_TYPE)) return;
	if (error) {
		free(error);
		error = NULL;
	}
	switch (dict_find(iter, KEY_TYPE)->value->uint8) {
		case KEY_TYPE_ERROR: {
			error = malloc(dict_find(iter, KEY_LABEL)->length);
			strncpy(error, dict_find(iter, KEY_LABEL)->value->cstring, dict_find(iter, KEY_LABEL)->length - 1);
			LOG("error: %s", error);
			all_menu_layer_reload_data_and_mark_dirty(menu_layer);
			break;
		}
		case KEY_TYPE_LIGHT:
			switch (dict_find(iter, KEY_METHOD)->value->uint8) {
				case KEY_METHOD_BEGIN:
					if (lights) free(lights);
					num_lights = dict_find(iter, KEY_INDEX)->value->uint8;
					lights = malloc(sizeof(Light) * num_lights);
					break;
				case KEY_METHOD_END:
					all_menu_layer_reload_data_and_mark_dirty(menu_layer);
					break;
				case KEY_METHOD_DATA: {
					uint8_t index = dict_find(iter, KEY_INDEX)->value->uint8;
					Light *light = &lights[index];
					light->index = index;
					strncpy(light->label, dict_find(iter, KEY_LABEL)->value->cstring, sizeof(light->label) - 1);
					strncpy(light->state, dict_find(iter, KEY_STATE)->value->cstring, sizeof(light->state) - 1);
					light->color = (Color) {
						.hue = dict_find(iter, KEY_COLOR_H)->value->uint8,
						.saturation = dict_find(iter, KEY_COLOR_S)->value->uint8,
						.brightness = dict_find(iter, KEY_COLOR_B)->value->uint8,
					};
					LOG("added light: %d '%s' '%s'", light->index, light->label, light->state);
					all_menu_layer_reload_data_and_mark_dirty(menu_layer);
					break;
				}
			}
			break;
		case KEY_TYPE_TAG:
			switch (dict_find(iter, KEY_METHOD)->value->uint8) {
				case KEY_METHOD_BEGIN:
					if (tags) free(tags);
					num_tags = dict_find(iter, KEY_INDEX)->value->uint8;
					tags = malloc(sizeof(Light) * num_tags);
					break;
				case KEY_METHOD_END:
					all_menu_layer_reload_data_and_mark_dirty(menu_layer);
					break;
				case KEY_METHOD_DATA: {
					uint8_t index = dict_find(iter, KEY_INDEX)->value->uint8;
					Light *tag = &tags[index];
					tag->index = index;
					strncpy(tag->label, dict_find(iter, KEY_LABEL)->value->cstring, sizeof(tag->label) - 1);
					strncpy(tag->state, "", sizeof(tag->state) - 1);
					tag->color = (Color) {
						.hue = dict_find(iter, KEY_COLOR_H)->value->uint8,
						.saturation = dict_find(iter, KEY_COLOR_S)->value->uint8,
						.brightness = dict_find(iter, KEY_COLOR_B)->value->uint8,
					};
					LOG("added tag: %d '%s' '%s'", tag->index, tag->label, tag->state);
					all_menu_layer_reload_data_and_mark_dirty(menu_layer);
					break;
				}
			}
			break;
	}
}

void lightlist_out_sent_handler(DictionaryIterator *sent) {
}

void lightlist_out_failed_handler(DictionaryIterator *failed, AppMessageResult reason) {
	if (error) free(error);
	error = malloc(sizeof(char) * 65);
	strncpy(error, "Unable to connect to phone! Make sure the Pebble app is running.", 64);
	LOG("error: %s", error);
	all_menu_layer_reload_data_and_mark_dirty(menu_layer);
}

bool lightlist_is_on_top() {
	return window == window_stack_get_top_window();
}

void lightlist_update_settings() {
	menu_section_lights = settings()->tags_first ? 2 : 1;
	menu_section_tags = settings()->tags_first ? 1 : 2;
	menu_section_lights = settings()->hide_lights ? 88 : menu_section_lights;
	menu_section_tags = settings()->hide_tags ? 89 : menu_section_tags;
	menu_layer_reload_data_and_mark_dirty(menu_layer);
}

Light* light() {
	switch (selected_type) {
		case KEY_TYPE_ALL:
			return &all_lights[0];
		case KEY_TYPE_LIGHT:
			return &lights[selected_index];
		case KEY_TYPE_TAG:
			return &tags[selected_index];
	}
	return NULL;
}

void light_toggle() {
	if (selected_type == KEY_TYPE_LIGHT)
		strncpy(light()->state, "...", sizeof(light()->state) - 1);
	all_menu_layer_reload_data_and_mark_dirty();
	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);
	if (iter == NULL)
		return;
	dict_write_uint8(iter, KEY_TYPE, selected_type);
	dict_write_uint8(iter, KEY_INDEX, selected_index);
	dict_write_end(iter);
	app_message_outbox_send();
}

void light_update_color() {
	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);
	if (iter == NULL)
		return;
	dict_write_uint8(iter, KEY_TYPE, selected_type);
	dict_write_uint8(iter, KEY_INDEX, selected_index);
	dict_write_uint8(iter, KEY_COLOR_H, light()->color.hue);
	dict_write_uint8(iter, KEY_COLOR_S, light()->color.saturation);
	dict_write_uint8(iter, KEY_COLOR_B, light()->color.brightness);
	dict_write_end(iter);
	app_message_outbox_send();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

static void all_menu_layer_reload_data_and_mark_dirty() {
	menu_layer_reload_data_and_mark_dirty(menu_layer);
	lightmenu_reload_data_and_mark_dirty();
}

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
	if (section_index == MENU_SECTION_ALL) {
		return MENU_CELL_BASIC_HEADER_HEIGHT;
	} else if (section_index == menu_section_lights) {
		return num_lights ? MENU_CELL_BASIC_HEADER_HEIGHT : 0;
	} else if (section_index == menu_section_tags) {
		return num_tags ? MENU_CELL_BASIC_HEADER_HEIGHT : 0;
	} else if (section_index == MENU_SECTION_OTHER) {
		return MENU_CELL_BASIC_HEADER_HEIGHT;
	} else {
		return 1;
	}
}

static int16_t menu_get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	if (cell_index->section == MENU_SECTION_ALL) {
		if (error) {
			return graphics_text_layout_get_content_size(error, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), (GRect) { .origin = { 4, 2 }, .size = { PEBBLE_WIDTH - 8, 88 } }, GTextOverflowModeFill, GTextAlignmentLeft).h + 12;
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
	if (section_index == MENU_SECTION_ALL) {
		menu_cell_basic_header_draw(ctx, cell_layer, "PebblFX");
	} else if (section_index == menu_section_lights) {
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
