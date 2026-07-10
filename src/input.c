#include "input.h"
#include "command.h"
#include "history.h"
#include <string.h>

void input_handle(App *a, int ch) {
    if (ch == ERR) return;
    switch (ch) {
        case '\n':
        case '\r':
            if (a->input_len > 0) {
                app_add_message(a, ROLE_USER, a->input_buf);
                history_log_user(a->input_buf);
                Command c;
                CmdType ct = command_parse(a->input_buf, &c);
                if (ct == CMD_NONE) {
                    const char *r = "## `429` Too many Requests\n\n**服务器繁忙，请稍后再试。**";
                    app_add_message(a, ROLE_ASSISTANT, r);
                    history_log_fixed_reply();
                } else {
                    command_execute(a, &c, a->input_buf);
                }
                a->input_buf[0] = '\0';
                a->input_len = 0;
            }
            break;
        case KEY_BACKSPACE:
        case 127:
        case '\b':
            if (a->input_len > 0) {
                a->input_len--;
                a->input_buf[a->input_len] = '\0';
            }
            break;
        case 3:
            a->input_buf[0] = '\0';
            a->input_len = 0;
            break;
        default:
            if (ch >= 32 && ch <= 126 && a->input_len < MAX_INPUT_LEN - 2) {
                a->input_buf[a->input_len++] = (char)ch;
                a->input_buf[a->input_len] = '\0';
            }
            break;
    }
}
