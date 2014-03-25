#pragma once

typedef struct {
	uint8_t hue;
	uint8_t saturation;
	uint8_t brightness;
} Color;

typedef struct {
	uint8_t index;
	char label[32];
	char state[5];
	Color color;
} Light;

enum {
	KEY_INDEX = 0x0,
	KEY_LABEL = 0x1,
	KEY_STATE = 0x2,
	KEY_COLOR_H = 0x3,
	KEY_COLOR_S = 0x4,
	KEY_COLOR_B = 0x5,
	KEY_TAG = 0x6,
	KEY_ERROR = 0x7,
};
