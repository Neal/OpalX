#pragma once

#include "../common.h"

void options_init(void);
void options_destroy(void);
void options_show(void);
void options_in_received_handler(DictionaryIterator *iter);
void options_out_sent_handler(DictionaryIterator *sent);
void options_out_failed_handler(DictionaryIterator *failed, AppMessageResult reason);
bool options_is_on_top();
