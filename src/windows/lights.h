#pragma once

#include "../common.h"

void lights_init(void);
void lights_destroy(void);
void lights_in_received_handler(DictionaryIterator *iter);
void lights_out_sent_handler(DictionaryIterator *sent);
void lights_out_failed_handler(DictionaryIterator *failed, AppMessageResult reason);
bool lights_is_on_top();
Light* light();
