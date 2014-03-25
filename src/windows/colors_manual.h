#pragma once

void colors_manual_init(void);
void colors_manual_destroy(void);
void colors_manual_show(void);
void colors_manual_in_received_handler(DictionaryIterator *iter);
void colors_manual_out_sent_handler(DictionaryIterator *sent);
void colors_manual_out_failed_handler(DictionaryIterator *failed, AppMessageResult reason);
bool colors_manual_is_on_top();
