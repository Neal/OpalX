#pragma once

#include "../common.h"

void colors_default_init(void);
void colors_default_destroy(void);
void colors_default_in_received_handler(DictionaryIterator *iter);
void colors_default_out_sent_handler(DictionaryIterator *sent);
void colors_default_out_failed_handler(DictionaryIterator *failed, AppMessageResult reason);
bool colors_default_is_on_top();
