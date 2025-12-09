#include <stdio.h>
#include <string.h> 
#include <stdlib.h> 
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <ctype.h>
#include <pwd.h>
#include <stdbool.h> 
#include <signal.h>
#include "process.h"

//F1: aide
void aide_process(){
    opt_help();
}

//F4: Rechercher un processus par son PID
process_t* recherche_process(process_t *list, int count, pid_t pid) {
    if (!list || count <= 0) {
        return NULL;
    }
    
    for (int i = 0; i < count; i++) {
        if (list[i].pid == pid) {
            return &list[i];  // Retourner un pointeur vers le processus trouvé
        }
    }
    
    return NULL;  // Processus non trouvé
}

// F5: Mettre en pause un processus
int pause_process(pid_t pid) {
    if (kill(pid, SIGSTOP) == -1) {
        perror("kill SIGSTOP");
        return -1;
    }
    return 0;
}

// F6: Arrêter un processus
int arret_process(pid_t pid) {
    if (kill(pid, SIGTERM) == -1) {
        perror("kill SIGTERM");
        return -1;
    }
    return 0;
}

// F7:Tuer un processus
int tuer_process(pid_t pid) {
    if (kill(pid, SIGKILL) == -1) {
        perror("kill SIGKILL");
        return -1;
    }
    return 0;
}

//F8: Redémarrer un processus
int redemarrer_process(pid_t pid) {
    if (kill(pid, SIGCONT) == -1) {
        perror("kill SIGCONT");
        return -1;
    }
    return 0;
}

int main(){
    return 0;
}