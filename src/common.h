#pragma once

typedef struct {
	int hue;
	int saturation;
	int brightness;
} Color;

typedef struct {
	int index;
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
	KEY_ERROR = 0x6,
};
