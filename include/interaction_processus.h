#ifndef INTERACTION_PROCESSUS_H
#define INTERACTION_PROCESSUS_H

#include "manager.h" 


void aide_process();
process_t* recherche_process(process_t *list, int count, pid_t pid);
int pause_process(pid_t pid);
int arret_process(pid_t pid);
int tuer_process(pid_t pid);
int redemarrer_process(pid_t pid);

#endif