#ifndef APP_H
#define APP_H
#include <ncurses.h>
#include <time.h>
#define MAX_MESSAGES 200
#define MAX_INPUT_LEN 4096
#define HISTORY_PATH "log/history.jsonl"
typedef enum { ROLE_USER, ROLE_ASSISTANT, ROLE_TOOL } Role;
typedef struct { Role role; char *text; } Message;
typedef struct {
    WINDOW *dialog_win, *status_win, *input_win;
    Message messages[MAX_MESSAGES];
    int msg_count;
    char input_buf[MAX_INPUT_LEN];
    int input_len;
    char cwd[256], cwd_display[256], time_str[16];
    int exit_flag;
} App;
void app_init(App*);
void app_cleanup(App*);
void app_add_message(App*, Role, const char*);
void app_run(App*);
#endif
