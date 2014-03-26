#pragma once

typedef struct {
	uint8_t hue;
	uint8_t saturation;
	uint8_t brightness;
} Color;

typedef struct {
	uint8_t index;
	char label[18];
	char state[5];
	Color color;
} Light;

enum {
	KEY_TYPE,
	KEY_METHOD,
	KEY_INDEX,
	KEY_LABEL,
	KEY_STATE,
	KEY_COLOR_H,
	KEY_COLOR_S,
	KEY_COLOR_B,
};

enum {
	KEY_TYPE_ERROR,
	KEY_TYPE_LIGHT,
	KEY_TYPE_TAG,
	KEY_TYPE_ALL,
};

enum {
	KEY_METHOD_BEGIN,
	KEY_METHOD_DATA,
	KEY_METHOD_END,
};
