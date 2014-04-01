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
	if (persist_exists(KEY_SETTINGS)) {
		int res = persist_read_data(KEY_SETTINGS, &_settings, sizeof(_settings));
		LOG("settings_load: %d", res);
	}
	else {
		LOG("settings_load: No settings.");
	}
}

Settings* settings() {
	return &_settings;
}

void settings_save(void) {
	int res = persist_write_data(KEY_SETTINGS, &_settings, sizeof(_settings));
	LOG("settings_save: %d", res);
}
