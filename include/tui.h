#ifndef TUI_H
#define TUI_H
#include "app.h"
void tui_init(App*);
void tui_cleanup(App*);
void tui_refresh_all(App*);
void tui_draw_status(App*);
void tui_draw_input(App*);
void tui_draw_dialog(App*);
#endif
