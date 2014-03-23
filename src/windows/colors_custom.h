#pragma once

#include "../common.h"

void colors_custom_init(void);
void colors_custom_destroy(void);
void colors_custom_in_received_handler(DictionaryIterator *iter);
void colors_custom_out_sent_handler(DictionaryIterator *sent);
void colors_custom_out_failed_handler(DictionaryIterator *failed, AppMessageResult reason);
bool colors_custom_is_on_top();
