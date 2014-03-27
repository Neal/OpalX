#include <pebble.h>
#include "appmessage.h"
#include "settings.h"
#include "windows/lightlist.h"

static void init(void) {
	appmessage_init();
	settings_load();
	lightlist_init();
}

static void deinit(void) {
	settings_save();
	lightlist_destroy();
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}
