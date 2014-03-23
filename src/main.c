#include <pebble.h>
#include "appmessage.h"
#include "windows/lights.h"

static void init(void) {
	appmessage_init();
	lights_init();
}

static void deinit(void) {
	lights_destroy();
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}
