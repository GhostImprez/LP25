#ifndef PROCESS_H
#define PROCESS_H

#include "manager.h" 

// Prototypes Process (Lecture /proc)
long read_total_cpu_ticks();
void update_local_processes(machine_t *m);

// Prototypes Interaction (Actions F5, F6...)
int pause_process(pid_t pid);
int arret_process(pid_t pid);
int tuer_process(pid_t pid);
int redemarrer_process(pid_t pid);
void aide_process();

#endif