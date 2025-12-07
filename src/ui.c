#include "ui.h"
#include <stdlib.h>
#include <string.h>
void ui_init(ui_ctx_t *c) { 
    initscr(); cbreak(); noecho(); keypad(stdscr,TRUE); curs_set(0); start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLACK); 
    init_pair(2, COLOR_BLACK, COLOR_CYAN); 
    init_pair(3, COLOR_BLACK, COLOR_GREEN); 
    getmaxyx(stdscr, c->h, c->w); c->tab=0; c->sel=0; c->scroll=0;
}
void ui_end(void) { endwin(); }
void ui_draw(ui_ctx_t *c, machine_t *m, int n) {
    erase();
    attron(COLOR_PAIR(2)); mvprintw(0,0,"%s", m[c->tab].name); for(int i=strlen(m[c->tab].name);i<c->w;i++) addch(' '); attroff(COLOR_PAIR(2));
    mvprintw(2,1,"PID   USER       CPU   MEM   CMD");
    for(int i=0; i<c->h-4; i++) {
        int idx = c->scroll + i;
        if(idx >= m[c->tab].count) break;
        if(idx == c->sel) attron(COLOR_PAIR(3));
        mvprintw(3+i, 1, "%-5d %-10s %-5.1f %-5.1f %s", m[c->tab].procs[idx].pid, m[c->tab].procs[idx].user, m[c->tab].procs[idx].cpu, m[c->tab].procs[idx].mem, m[c->tab].procs[idx].cmd);
        attroff(COLOR_PAIR(3));
    }
    mvprintw(c->h-1, 1, "F2:Next Tab  q:Quit");
    refresh();
}
int ui_input(ui_ctx_t *c, int n, int count) {
    int k = getch();
    if(k == KEY_F(2)) { c->tab = (c->tab + 1) % n; c->sel=0; }
    if(k == KEY_DOWN && c->sel < count-1) c->sel++;
    if(k == KEY_UP && c->sel > 0) c->sel--;
    return k;
}
