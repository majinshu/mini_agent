#ifndef TOOLS_H
#define TOOLS_H
#include "app.h"
typedef enum {
    TOOL_OK, TOOL_NOT_FOUND, TOOL_PERMISSION_DENIED,
    TOOL_TOO_MANY_LINES, TOOL_TIMEOUT, TOOL_ERROR
} ToolResult;
typedef struct {
    ToolResult result;
    char output[16384];
    int exit_code, timed_out;
} ToolOutput;
void tool_read_file(const char*, int, int, ToolOutput*);
void tool_write_file(const char*, const char*, ToolOutput*);
void tool_exec_cmd(const char*, ToolOutput*);
#endif
