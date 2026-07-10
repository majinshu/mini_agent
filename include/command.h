#ifndef COMMAND_H
#define COMMAND_H
#include "app.h"
typedef enum { CMD_READ, CMD_WRITE, CMD_EXEC, CMD_EXIT, CMD_UNKNOWN, CMD_NONE } CmdType;
typedef struct {
    CmdType type;
    char arg1[512], arg2[2048];
    int line_start, line_end, has_line_range;
} Command;
CmdType command_parse(const char*, Command*);
void command_execute(App*, const Command*, const char*);
#endif
