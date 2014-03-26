#include <pebble.h>
#include "appmessage.h"
#include "common.h"
#include "windows/lightlist.h"
#include "windows/lightmenu.h"
#include "windows/colors_custom.h"
#include "windows/colors_default.h"
#include "windows/colors_manual.h"

static void in_received_handler(DictionaryIterator *iter, void *context);
static void in_dropped_handler(AppMessageResult reason, void *context);
static void out_sent_handler(DictionaryIterator *sent, void *context);
static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context);

void appmessage_init(void) {
	app_message_register_inbox_received(in_received_handler);
	app_message_register_inbox_dropped(in_dropped_handler);
	app_message_register_outbox_sent(out_sent_handler);
	app_message_register_outbox_failed(out_failed_handler);
	app_message_open(256 /* inbound_size */, 256 /* outbound_size */);
}

static void in_received_handler(DictionaryIterator *iter, void *context) {
	if (lightlist_is_on_top()) {
		lightlist_in_received_handler(iter);
	} else if (lightmenu_is_on_top()) {
		lightmenu_in_received_handler(iter);
	} else if (colors_custom_is_on_top()) {
		colors_custom_in_received_handler(iter);
	} else if (colors_default_is_on_top()) {
		colors_default_in_received_handler(iter);
	} else if (colors_manual_is_on_top()) {
		colors_manual_in_received_handler(iter);
	}
}

static void in_dropped_handler(AppMessageResult reason, void *context) {
}

static void out_sent_handler(DictionaryIterator *sent, void *context) {
	if (lightlist_is_on_top()) {
		lightlist_out_sent_handler(sent);
	} else if (lightmenu_is_on_top()) {
		lightmenu_out_sent_handler(sent);
	} else if (colors_custom_is_on_top()) {
		colors_custom_out_sent_handler(sent);
	} else if (colors_default_is_on_top()) {
		colors_default_out_sent_handler(sent);
	} else if (colors_manual_is_on_top()) {
		colors_manual_out_sent_handler(sent);
	}
}

static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
	if (lightlist_is_on_top()) {
		lightlist_out_failed_handler(failed, reason);
	} else if (lightmenu_is_on_top()) {
		lightmenu_out_failed_handler(failed, reason);
	} else if (colors_custom_is_on_top()) {
		colors_custom_out_failed_handler(failed, reason);
	} else if (colors_default_is_on_top()) {
		colors_default_out_failed_handler(failed, reason);
	} else if (colors_manual_is_on_top()) {
		colors_manual_out_failed_handler(failed, reason);
	}
}
