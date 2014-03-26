#pragma once

#include "../common.h"

void lightmenu_init(void);
void lightmenu_destroy(void);
void lightmenu_show(void);
void lightmenu_in_received_handler(DictionaryIterator *iter);
void lightmenu_out_sent_handler(DictionaryIterator *sent);
void lightmenu_out_failed_handler(DictionaryIterator *failed, AppMessageResult reason);
bool lightmenu_is_on_top();
