#pragma once

typedef struct {
	int index;
	char label[32];
	char color[32];
	char state[5];
} Light;

enum {
	KEY_INDEX = 0x0,
	KEY_LABEL = 0x1,
	KEY_COLOR = 0x2,
	KEY_STATE = 0x3,
	KEY_ERROR = 0x4,
};
