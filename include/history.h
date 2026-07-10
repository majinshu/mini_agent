#ifndef HISTORY_H
#define HISTORY_H
#include "app.h"
void history_log_user(const char*);
void history_log_tool_call(const char*, const char*);
void history_log_tool_result(const char*, int, const char*, const char*);
void history_log_fixed_reply(void);
void history_load(App*);
#endif
