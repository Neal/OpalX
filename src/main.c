#include <pebble.h>
#include "appmessage.h"
#include "windows/lightlist.h"

static void init(void) {
	appmessage_init();
	lightlist_init();
}

static void deinit(void) {
	lightlist_destroy();
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}
