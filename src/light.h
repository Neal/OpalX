#pragma once

typedef struct {
	uint8_t hue;
	uint8_t saturation;
	uint8_t brightness;
	uint16_t kelvin;
} Color;

typedef struct {
	uint8_t index;
	char label[18];
	char state[5];
	Color color;
} Light;

extern Light* all_lights;
extern Light* lights;
extern Light* tags;
extern char* error;
extern uint8_t num_lights;
extern uint8_t num_tags;
extern uint8_t selected_index;
extern uint8_t selected_type;
extern uint8_t menu_section_lights;
extern uint8_t menu_section_tags;

void light_init(void);
void light_deinit(void);
void light_in_received_handler(DictionaryIterator *iter);
void light_out_sent_handler(DictionaryIterator *sent);
void light_out_failed_handler(DictionaryIterator *failed, AppMessageResult reason);
void light_update_settings();
void light_refresh();
void light_toggle();
void light_on();
void light_off();
void light_update_color();
void all_menu_layer_reload_data_and_mark_dirty();
Light* light();
