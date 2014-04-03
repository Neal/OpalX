#include <pebble.h>
#include "settings.h"
#include "common.h"
#include "libs/pebble-assist.h"

Settings _settings = {
	.hide_lights = false,
	.hide_tags = false,
	.tags_first = true,
};

void settings_load(void) {
	int res = persist_exists(KEY_SETTINGS) ? persist_read_data(KEY_SETTINGS, &_settings, sizeof(_settings)) : 0;
	LOG("settings_load: %d", res);
}

Settings* settings() {
	return &_settings;
}

void settings_save(void) {
	int res = persist_write_data(KEY_SETTINGS, &_settings, sizeof(_settings));
	LOG("settings_save: %d", res);
}
