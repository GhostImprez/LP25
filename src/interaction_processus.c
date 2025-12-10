#define _POSIX_C_SOURCE 200809L 
#include <stdio.h>
#include <string.h> 
#include <stdlib.h> 
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include "interaction_processus.h"

// F1: Aide (Remplace opt_help qui n'existe pas ici)
void aide_process() {
    printf("[INFO] Aide : Utilisez les flèches pour naviguer, F5-F8 pour agir sur les processus.\n");
}

// F4: Rechercher un processus par son PID
process_t* recherche_process(process_t *list, int count, pid_t pid) {
        if (!list || count <= 0) {
        return NULL;
    }
    
    for (int i = 0; i < count; i++) {
        if (list[i].pid == pid) {
            return &list[i];  // Retourne un pointeur vers le processus trouvé
        }
    }
    return NULL;  // Processus non trouvé
}

// F5: Mettre en pause un processus (SIGSTOP)
int pause_process(pid_t pid) {
    if (kill(pid, SIGSTOP) == -1) {
        perror("Erreur pause (SIGSTOP)");
        return -1;
    }
    return 0;
}

// F6: Arrêter un processus proprement (SIGTERM)
int arret_process(pid_t pid) {
    if (kill(pid, SIGTERM) == -1) {
        perror("Erreur arrêt (SIGTERM)");
        return -1;
    }
    return 0;
}

// F7: Tuer un processus immédiatement (SIGKILL)
int tuer_process(pid_t pid) {
    if (kill(pid, SIGKILL) == -1) {
        perror("Erreur kill (SIGKILL)");
        return -1;
    }
    return 0;
}

// F8: Redémarrer/Reprendre un processus (SIGCONT)
int redemarrer_process(pid_t pid) {
    if (kill(pid, SIGCONT) == -1) {
        perror("Erreur reprise (SIGCONT)");
        return -1;
    }
    return 0;
}

