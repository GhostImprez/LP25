#include "ui.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
int main() {
    ui_ctx_t ctx; machine_t m[2];
    sprintf(m[0].name, "LOCAL"); m[0].count=10; m[0].procs=malloc(sizeof(proc_t)*10);
    for(int i=0;i<10;i++) { m[0].procs[i].pid=i; sprintf(m[0].procs[i].user,"root"); sprintf(m[0].procs[i].cmd,"process_%d",i); }
    sprintf(m[1].name, "DISTANT"); m[1].count=5; m[1].procs=malloc(sizeof(proc_t)*5);
    for(int i=0;i<5;i++) { m[1].procs[i].pid=i*100; sprintf(m[1].procs[i].user,"admin"); sprintf(m[1].procs[i].cmd,"remote_%d",i); }
    ui_init(&ctx);
    while(ui_input(&ctx, 2, m[ctx.tab].count) != 'q') ui_draw(&ctx, m, 2);
    ui_end();
    return 0;
}
