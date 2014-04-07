#include <pebble.h>
#include "light.h"
#include "libs/pebble-assist.h"
#include "common.h"
#include "settings.h"
#include "windows/lightlist.h"

static void timer_callback(void *data);
static bool heard_from_js = false;
static AppTimer *timer;

Light* all_lights;
Light* lights;
Light* tags;
char* error;
uint8_t num_lights;
uint8_t num_tags;
uint8_t selected_index;
uint8_t selected_type;
uint8_t menu_section_lights;
uint8_t menu_section_tags;

void light_init(void) {
	timer = app_timer_register(1000, timer_callback, NULL);

	all_lights = malloc(sizeof(Light));
	all_lights->index = 0;
	strncpy(all_lights->label, "All Lights", sizeof(all_lights->label) - 1);
	all_lights->color = (Color) {
		.hue = 50,
		.saturation = 100,
		.brightness = 100,
	};

	lightlist_init();
	light_update_settings();
}

void light_deinit(void) {
	if (error) free(error);
	if (all_lights) free(all_lights);
	if (lights) free(lights);
	if (tags) free(tags);
	lightlist_deinit();
}

void light_in_received_handler(DictionaryIterator *iter) {
	heard_from_js = true;
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
			all_menu_layer_reload_data_and_mark_dirty();
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
					all_menu_layer_reload_data_and_mark_dirty();
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
					LOG("light: %d '%s' '%s' %d %d %d", light->index, light->label, light->state, light->color.hue, light->color.saturation, light->color.brightness);
					all_menu_layer_reload_data_and_mark_dirty();
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
					all_menu_layer_reload_data_and_mark_dirty();
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
					LOG("tag: %d '%s' '%s' %d %d %d", tag->index, tag->label, tag->state, tag->color.hue, tag->color.saturation, tag->color.brightness);
					all_menu_layer_reload_data_and_mark_dirty();
					break;
				}
			}
			break;
	}
}

void light_out_sent_handler(DictionaryIterator *sent) {
}

void light_out_failed_handler(DictionaryIterator *failed, AppMessageResult reason) {
	if (error) free(error);
	error = malloc(sizeof(char) * 65);
	strncpy(error, "Unable to connect to phone! Make sure the Pebble app is running.", 64);
	LOG("error: %s", error);
	all_menu_layer_reload_data_and_mark_dirty();
}

void light_update_settings() {
	menu_section_lights = settings()->tags_first ? 2 : 1;
	menu_section_tags = settings()->tags_first ? 1 : 2;
	menu_section_lights = settings()->hide_lights ? 88 : menu_section_lights;
	menu_section_tags = settings()->hide_tags ? 89 : menu_section_tags;
	all_menu_layer_reload_data_and_mark_dirty();
}

void light_refresh() {
	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);
	dict_write_uint8(iter, KEY_METHOD, KEY_METHOD_REFRESH);
	dict_write_end(iter);
	app_message_outbox_send();
}

void light_toggle() {
	if (selected_type == KEY_TYPE_LIGHT)
		strncpy(light()->state, "...", sizeof(light()->state) - 1);
	all_menu_layer_reload_data_and_mark_dirty();
	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);
	dict_write_uint8(iter, KEY_METHOD, KEY_METHOD_TOGGLE);
	dict_write_uint8(iter, KEY_TYPE, selected_type);
	dict_write_uint8(iter, KEY_INDEX, selected_index);
	dict_write_end(iter);
	app_message_outbox_send();
}

void light_update_color() {
	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);
	dict_write_uint8(iter, KEY_METHOD, KEY_METHOD_COLOR);
	dict_write_uint8(iter, KEY_TYPE, selected_type);
	dict_write_uint8(iter, KEY_INDEX, selected_index);
	dict_write_uint8(iter, KEY_COLOR_H, light()->color.hue);
	dict_write_uint8(iter, KEY_COLOR_S, light()->color.saturation);
	dict_write_uint8(iter, KEY_COLOR_B, light()->color.brightness);
	dict_write_end(iter);
	app_message_outbox_send();
}

void all_menu_layer_reload_data_and_mark_dirty() {
	lightlist_reload_data_and_mark_dirty();
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

static void timer_callback(void *data) {
	if (!heard_from_js) {
		DictionaryIterator *iter;
		app_message_outbox_begin(&iter);
		dict_write_uint8(iter, KEY_METHOD, KEY_METHOD_READY);
		dict_write_end(iter);
		app_message_outbox_send();
	}
}
