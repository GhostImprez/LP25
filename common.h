#ifndef COMMON_H
#define COMMON_H

#include <sys/types.h>

#define MAX_CMD_LEN 256
#define MAX_PROCESSES 1024  // Limite fixe pour le MVP pour éviter les realloc complexes tout de suite

typedef struct {
    pid_t pid;
    char user[32];
    char command[MAX_CMD_LEN];
    char state;             // 'R' (Running), 'S' (Sleeping), 'Z' (Zombie), etc.
    double cpu_usage;       // Pourcentage
    double mem_usage;       // Pourcentage
} process_info_t;

typedef struct {
    process_info_t list[MAX_PROCESSES];
    int count;              // Nombre actuel de processus
    bool running;           // Pour contrôler la boucle principale
    int selected_idx;       // Index du processus sélectionné dans l'UI
} app_state_t;

#endif 