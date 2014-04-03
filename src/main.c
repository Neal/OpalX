#include <pebble.h>
#include "appmessage.h"
#include "settings.h"
#include "light.h"

static void init(void) {
	appmessage_init();
	settings_load();
	light_init();
}

static void deinit(void) {
	settings_save();
	light_deinit();
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}
