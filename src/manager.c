#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "manager.h"

void manager_init(manager_t *m) {
    if (!m) return;
    memset(m, 0, sizeof(manager_t));
    m->running = true;
    // allocation initiale pour quelques machines
    m->machine_capacity = 4;
    m->machines  = calloc(m->machine_capacity, sizeof(machine_t));
    m->machine_count = 0;
}

void manager_clean(manager_t *m) {
    if (m->machines) {
        // Nettoyage de chaque machine
        for (int i = 0; i < m->machine_count; i++) {
            if (m->machines[i].name) free(m->machines[i].name);
            if (m->machines[i].host) free(m->machines[i].host);
            if (m->machines[i].user) free(m->machines[i].user);
            if (m->machines[i].password) free(m->machines[i].password);
            // Nettoyage de la liste des processus
            if (m->machines[i].processes.list) {
                free(m->machines[i].processes.list);
            }
        }
    }
    free(m->machines);
}

int manager_add_machine(manager_t *m, const char *name, const char *host, int port, conn_type_e type) {
    if (m->machine_count >= m->machine_capacity) {
        //aggrandir le tableau
        m->machine_capacity *= 2;
        m->machines = realloc(m->machines, m->machine_capacity * sizeof(machine_t));
        if (!m->machines) return -1; // échec
    }
    machine_t *new_m = &m->machines[m->machine_count];
    //initialisation
    memset(new_m, 0, sizeof(machine_t));
    new_m->name = strdup(name ? name : "Inconnu");
    new_m->host = host ? strdup(host) : NULL;
    new_m->port = port;
    new_m->type = type;
    new_m->connected = true; //toujours connecté pour le local

    m->machine_count++;
    return 0;
}

