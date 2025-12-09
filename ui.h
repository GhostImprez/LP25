#ifndef UI_H
#define UI_H
#include <ncurses.h>
typedef struct { int pid; char user[32]; double cpu; double mem; char state; char cmd[256]; } proc_t;
typedef struct { char name[64]; proc_t *procs; int count; } machine_t;
typedef struct { int h; int w; int tab; int sel; int scroll; } ui_ctx_t;
void ui_init(ui_ctx_t *c);
void ui_end(void);
void ui_draw(ui_ctx_t *c, machine_t *m, int n);
int ui_input(ui_ctx_t *c, int n, int count);
#endif
