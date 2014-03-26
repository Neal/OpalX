#pragma once

#include "../common.h"

void lightlist_init(void);
void lightlist_destroy(void);
void lightlist_in_received_handler(DictionaryIterator *iter);
void lightlist_out_sent_handler(DictionaryIterator *sent);
void lightlist_out_failed_handler(DictionaryIterator *failed, AppMessageResult reason);
bool lightlist_is_on_top();
Light* light();
void light_toggle();
void light_update_color();
