#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "manager.h"

void manager_init(manager_t *m) {
    if (!m) return;
    
    // Initialisation de toute la structure à zéro 
    memset(m, 0, sizeof(manager_t));
    m->running = true;
    
    // On alloue un petit espace de départ pour éviter de réallouer 
    m->machine_capacity = 4;
    m->machines = calloc(m->machine_capacity, sizeof(machine_t));
    m->machine_count = 0;
}

void manager_clean(manager_t *m) {
    if (m->machines) {
        // On libère la mémoire pour chaque machine
        for (int i = 0; i < m->machine_count; i++) {
            if (m->machines[i].name) free(m->machines[i].name);
            if (m->machines[i].host) free(m->machines[i].host);
            if (m->machines[i].user) free(m->machines[i].user);
            if (m->machines[i].password) free(m->machines[i].password);
            
            // On demande aussi la libération de la liste des processus
            if (m->machines[i].processes.list) {
                free(m->machines[i].processes.list);
            }
        }
        // On libère le tableau des machines
        free(m->machines);
    }
}

int manager_add_machine(manager_t *m, const char *name, const char *host, int port, conn_type_e type) {
    // Si le tableau est plein on double sa taille
    if (m->machine_count >= m->machine_capacity) {
        m->machine_capacity *= 2;
        machine_t *tmp = realloc(m->machines, m->machine_capacity * sizeof(machine_t));
        if (!tmp) return -1; // Erreur d'allocation
        m->machines = tmp;
    }

    machine_t *new_m = &m->machines[m->machine_count];
    memset(new_m, 0, sizeof(machine_t));
    
    // Copie des informations (strdup alloue de la mémoire)
    new_m->name = strdup(name ? name : "Inconnu");
    new_m->host = host ? strdup(host) : NULL;
    new_m->port = port;
    new_m->type = type;
    new_m->connected = true; // Par défaut connecté (pour le local)

    m->machine_count++;
    return 0;
}

