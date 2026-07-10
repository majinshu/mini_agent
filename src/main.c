#include "app.h"
int main(void) {
    App a;
    app_init(&a);
    app_run(&a);
    app_cleanup(&a);
    return 0;
}
