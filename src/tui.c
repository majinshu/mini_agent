#include "tui.h"
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

void tui_init(App *a) {
    initscr();
    raw();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(1);
    start_color();
    init_pair(1, COLOR_GREEN, -1);
    init_pair(2, COLOR_RED, -1);
    init_pair(3, COLOR_BLUE, -1);
    init_pair(4, COLOR_BLACK, COLOR_WHITE);
    if (!getcwd(a->cwd, sizeof(a->cwd)))
        snprintf(a->cwd, sizeof(a->cwd), "/");
    const char *home = getenv("HOME");
    if (!home) home = "/root";
    int hl = strlen(home);
    if (strncmp(a->cwd, home, hl) == 0 &&
        (a->cwd[hl] == '/' || a->cwd[hl] == '\0'))
        snprintf(a->cwd_display, sizeof(a->cwd_display), "~%s", a->cwd + hl);
    else
        snprintf(a->cwd_display, sizeof(a->cwd_display), "%s", a->cwd);
    time_t t = time(NULL);
    strftime(a->time_str, sizeof(a->time_str), "%T", localtime(&t));
    int r, c;
    getmaxyx(stdscr, r, c);
    a->input_win = newwin(3, c, r-3, 0);
    a->status_win = newwin(1, c, r-4, 0);
    a->dialog_win = newwin(r-4, c, 0, 0);
    scrollok(a->dialog_win, TRUE);
}

void tui_cleanup(App *a) {
    delwin(a->dialog_win);
    delwin(a->status_win);
    delwin(a->input_win);
    endwin();
}

void tui_draw_status(App *a) {
    werase(a->status_win);
    int c = getmaxx(a->status_win);
    time_t t = time(NULL);
    strftime(a->time_str, sizeof(a->time_str), "%T", localtime(&t));
    wattron(a->status_win, COLOR_PAIR(4));
    for (int i = 0; i < c; i++)
        mvwaddch(a->status_win, 0, i, ' ');
    wattron(a->status_win, COLOR_PAIR(1));
    mvwprintw(a->status_win, 0, 1, "%s", a->cwd_display);
    wattroff(a->status_win, COLOR_PAIR(1));
    int tx = c - (int)strlen(a->time_str) - 1;
    if (tx < 0) tx = 0;
    mvwprintw(a->status_win, 0, tx, "%s", a->time_str);
    wattroff(a->status_win, COLOR_PAIR(4));
    wrefresh(a->status_win);
}

void tui_draw_input(App *a) {
    werase(a->input_win);
    int c = getmaxx(a->input_win);
    mvwprintw(a->input_win, 1, 1, "> %s", a->input_buf);
    int cx = 3 + a->input_len;
    if (cx >= c) cx = c - 1;
    wmove(a->input_win, 1, cx);
    wrefresh(a->input_win);
}

void tui_draw_dialog(App *a) {
    werase(a->dialog_win);
    int my, mx;
    getmaxyx(a->dialog_win, my, mx);
    (void)mx;
    int row = 0;
    int start = a->msg_count - my/2;
    if (start < 0) start = 0;
    for (int i = start; i < a->msg_count && row < my; i++) {
        Message *m = &a->messages[i];
        switch (m->role) {
            case ROLE_USER:
                wattron(a->dialog_win, COLOR_PAIR(1)|A_BOLD);
                mvwprintw(a->dialog_win, row++, 0, "User:");
                wattroff(a->dialog_win, COLOR_PAIR(1)|A_BOLD);
                break;
            case ROLE_ASSISTANT:
                wattron(a->dialog_win, COLOR_PAIR(2)|A_BOLD);
                mvwprintw(a->dialog_win, row++, 0, "Assistant:");
                wattroff(a->dialog_win, COLOR_PAIR(2)|A_BOLD);
                break;
            case ROLE_TOOL:
                wattron(a->dialog_win, COLOR_PAIR(3)|A_BOLD);
                mvwprintw(a->dialog_win, row++, 0, "Tool:");
                wattroff(a->dialog_win, COLOR_PAIR(3)|A_BOLD);
                break;
        }
        if (row < my) {
            mvwprintw(a->dialog_win, row, 0, "%s", m->text);
            row++;
        }
        if (row < my) row++;
    }
    wrefresh(a->dialog_win);
}

void tui_refresh_all(App *a) {
    tui_draw_status(a);
    tui_draw_dialog(a);
    tui_draw_input(a);
}
