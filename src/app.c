#include "app.h"
#include "tui.h"
#include "input.h"
#include "history.h"
#include <stdlib.h>
#include <string.h>

/* 初始化：清空结构体，启动界面，加载历史 */
void app_init(App *a) {
    memset(a, 0, sizeof(*a));
    tui_init(a);
    history_load(a);
}

/* 清理：释放消息内存，关闭 ncurses */
void app_cleanup(App *a) {
    for (int i = 0; i < a->msg_count; i++)
        free(a->messages[i].text);
    tui_cleanup(a);
}

/* 添加一条消息，满了就扔掉最旧的 */
void app_add_message(App *a, Role r, const char *t) {
    if (a->msg_count >= MAX_MESSAGES) {
        free(a->messages[0].text);
        for (int i = 1; i < MAX_MESSAGES; i++)
            a->messages[i-1] = a->messages[i];
        a->msg_count--;
    }
    Message *m = &a->messages[a->msg_count++];
    m->role = r;
    m->text = strdup(t ? t : "");
}

/* 主循环：刷新界面 → 等按键 → 处理 */
void app_run(App *a) {
    while (!a->exit_flag) {
        tui_refresh_all(a);
        int ch = getch();
        if (ch != ERR) input_handle(a, ch);
    }
}
