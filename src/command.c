#include "command.h"
#include "tools.h"
#include "history.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* 解析 "2-5" 或 "3" 这种行号范围 */
static int parse_range(const char *s, int *st, int *ed) {
    if (!s || !*s) return 0;
    if (sscanf(s, "%d-%d", st, ed) == 2 && *st > 0 && *ed >= *st) return 1;
    if (sscanf(s, "%d", st) == 1 && *st > 0) { *ed = *st; return 1; }
    return 0;
}

/* 解析 /xxx 命令，提取命令名和参数 */
CmdType command_parse(const char *in, Command *c) {
    memset(c, 0, sizeof(*c));
    c->type = CMD_NONE;
    if (!in || in[0] != '/') return CMD_NONE;
    const char *p = in + 1;
    while (*p == ' ') p++;
    char nm[64];
    int i = 0;
    while (*p && *p != ' ' && i < 63) nm[i++] = *p++;
    nm[i] = '\0';
    while (*p == ' ') p++;
    if (strcmp(nm, "exit") == 0) { c->type = CMD_EXIT; return CMD_EXIT; }
    if (strcmp(nm, "read") == 0) {
        c->type = CMD_READ;
        const char *a = p;
        while (*p && *p != ' ') p++;
        int l = p - a;
        if (l >= (int)sizeof(c->arg1)) l = sizeof(c->arg1)-1;
        strncpy(c->arg1, a, l); c->arg1[l] = '\0';
        while (*p == ' ') p++;
        if (*p) {
            strncpy(c->arg2, p, sizeof(c->arg2)-1);
            c->arg2[sizeof(c->arg2)-1] = '\0';
            c->has_line_range = parse_range(p, &c->line_start, &c->line_end);
        }
        return CMD_READ;
    }
    if (strcmp(nm, "write") == 0) {
        c->type = CMD_WRITE;
        const char *a = p;
        while (*p && *p != ' ') p++;
        int l = p - a;
        if (l >= (int)sizeof(c->arg1)) l = sizeof(c->arg1)-1;
        strncpy(c->arg1, a, l); c->arg1[l] = '\0';
        while (*p == ' ') p++;
        if (*p) {
            strncpy(c->arg2, p, sizeof(c->arg2)-1);
            c->arg2[sizeof(c->arg2)-1] = '\0';
        }
        return CMD_WRITE;
    }
    if (strcmp(nm, "exec") == 0) {
        c->type = CMD_EXEC;
        if (*p) {
            strncpy(c->arg1, p, sizeof(c->arg1)-1);
            c->arg1[sizeof(c->arg1)-1] = '\0';
        }
        return CMD_EXEC;
    }
    c->type = CMD_UNKNOWN;
    return CMD_UNKNOWN;
}

/* 执行 /read：读文件，显示在对话区 */
static void cmd_read(App *a, const Command *c) {
    ToolOutput o;
    tool_read_file(c->arg1, c->has_line_range ? c->line_start : -1,
                   c->has_line_range ? c->line_end : -1, &o);
    char m[256];
    snprintf(m, sizeof(m), "Tool_Use [read_file] file=%s", c->arg1);
    if (c->has_line_range)
        snprintf(m+strlen(m), sizeof(m)-strlen(m), " lines=%d-%d",
                 c->line_start, c->line_end);
    app_add_message(a, ROLE_ASSISTANT, m);
    if (o.result == TOOL_OK) {
        snprintf(m, sizeof(m), "File %s Read success, Line %d-%d",
                 c->arg1, c->line_start>0?c->line_start:1,
                 c->line_end>0?c->line_end:100);
        app_add_message(a, ROLE_TOOL, m);
        app_add_message(a, ROLE_TOOL, o.output);
    } else {
        snprintf(m, sizeof(m), "File %s Read failed, %s", c->arg1, o.output);
        app_add_message(a, ROLE_TOOL, m);
    }
}

/* /write：追加内容到文件 */
static void cmd_write(App *a, const Command *c) {
    ToolOutput o;
    tool_write_file(c->arg1, c->arg2, &o);
    char m[256];
    snprintf(m, sizeof(m), "Tool_Use [write_file] file=%s", c->arg1);
    app_add_message(a, ROLE_ASSISTANT, m);
    snprintf(m, sizeof(m), "File %s Write %s", c->arg1,
             o.result==TOOL_OK ? "success" : "failed");
    app_add_message(a, ROLE_TOOL, m);
}

/* /exec：执行 shell 命令，10秒超时 */
static void cmd_exec(App *a, const Command *c) {
    ToolOutput o;
    tool_exec_cmd(c->arg1, &o);
    char m[256];
    snprintf(m, sizeof(m), "Tool_Use [exec_cmd] cmd=%s", c->arg1);
    app_add_message(a, ROLE_ASSISTANT, m);
    if (o.timed_out)
        snprintf(m, sizeof(m), "Command timeout after 10s");
    else
        snprintf(m, sizeof(m), "Command exited with code %d", o.exit_code);
    app_add_message(a, ROLE_TOOL, m);
    if (o.output[0])
        app_add_message(a, ROLE_TOOL, o.output);
}

/* 根据命令类型分发 */
void command_execute(App *a, const Command *c, const char *r) {
    (void)r;
    switch (c->type) {
        case CMD_EXIT: a->exit_flag = 1; break;
        case CMD_READ: cmd_read(a, c); break;
        case CMD_WRITE: cmd_write(a, c); break;
        case CMD_EXEC: cmd_exec(a, c); break;
        case CMD_UNKNOWN: app_add_message(a, ROLE_TOOL, "Unknown command"); break;
        default: break;
    }
}
