#pragma once

typedef struct {
	bool hide_lights;
	bool hide_tags;
	bool tags_first;
} Settings;

Settings* settings();
void settings_load(void);
void settings_save(void);
