#define _POSIX_C_SOURCE 200809L // Requis pour usleep/strdup
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include "manager.h"
//#include "ui.h"
//#include "process.h"
//en attendant
void update_local_processes(machine_t *m) {
    // On simule une mise à jour des processus
    if (m->processes.count == 0) {
        m->processes.capacity = 10;
        m->processes.list = malloc(sizeof(process_t) * m->processes.capacity);
        m->processes.count = 3;
        
        // Faux processus 1
        m->processes.list[0].pid = 101;
        strcpy(m->processes.list[0].command, "systemd");
        strcpy(m->processes.list[0].user, "root");
        m->processes.list[0].cpu_usage = 0.1;
        
        // Faux processus 2
        m->processes.list[1].pid = 2042;
        strcpy(m->processes.list[1].command, "lp25_top");
        strcpy(m->processes.list[1].user, "etu");
        m->processes.list[1].cpu_usage = 12.5;
    }
    // Simulation variation CPU
    m->processes.list[1].cpu_usage += 0.5;
    if (m->processes.list[1].cpu_usage > 100.0) m->processes.list[1].cpu_usage = 0.0;
}

void ui_init() { printf("\n[UI] Interface Initialisée (ncurses)\n"); }
void ui_clean() { printf("[UI] Interface Nettoyée\n"); }
void ui_refresh(manager_t *m) {
    if (m->machine_count > 0) {
        printf("[UI] %s - Processus actifs: %d\n", 
               m->machines[0].name,
               m->machines[0].processes.count);
        for (int i = 0; i < m->machines[0].processes.count; i++) {
            process_t *p = &m->machines[0].processes.list[i];
            printf("  PID %d: %s (user: %s) CPU: %.1f%% MEM: %.1f%%\n",
                   p->pid, p->command, p->user, p->cpu_usage, p->mem_usage);
        }
        printf("---\n");
    }
}
int ui_get_input() { return 0; } // Pas de touche pressée


// Définition des options longues du getopt long
static struct option options_longues[] = {
    {"help", no_argument, 0, 'h'},                      // nécessaire pour local
    {"dry-run", no_argument, 0, 0},                     // nécessaire pour local    
    {"remote-config", required_argument, 0, 'c'},       
    {"connexion-type", required_argument, 0, 't'},
    {"port", required_argument, 0, 'P'},
    {"login", required_argument, 0, 'l'},               // nécessaire pour local  
    {"remote-server", required_argument, 0, 's'},
    {"username", required_argument, 0, 'u'},
    {"password", required_argument, 0, 'p'},
    {"all", no_argument, 0, 'a'},                       // nécessaire pour local
    {0, 0, 0, 0}
};

int main(int argc, char *argv[])
{
    manager_t mgr;
    manager_init(&mgr);
    
    // Parsing des arguments
    int opt, idx;
    while ((opt = getopt_long(argc, argv, "hc:l:a", options_longues, &idx)) != -1) {
        switch (opt) {
            case 0: // dry-run 
                if (strcmp(options_longues[idx].name, "dry-run") == 0) {
                    mgr.dry_run = true;
                }
                break;
            case 'h':
                printf("Usage: %s [OPTIONS]\n", argv[0]);
                manager_clean(&mgr);
                return 0;
            case 'c':
                // TODO : charger config distante - fonction manager_load_config  
                printf("[INFO] config fichier distant: %s\n", optarg);
                break;
            case 'a':
                mgr.show_all = true;
                break;
        }
    }

    // Setup mode local 
    if (mgr.machine_count == 0) {
        manager_add_machine(&mgr, "Systeme local", "127.0.0.1", 0, CONN_LOCAL);
    }

    //initilisation interface graphique
    if (!mgr.dry_run) {
        ui_init();
    }

    //boucle principale
    while (mgr.running){
        // 1 - Mise à jour des processus
        for(int i = 0; i < mgr.machine_count; i++) {
            if (mgr.machines[i].type == CONN_LOCAL) {
                // appel au module process (Enzo)
                update_local_processes(&mgr.machines[i]);
            }
        }

        // 2 - Affichage UI
        if (!mgr.dry_run) {
            ui_refresh(&mgr);
        }
        else {
            printf("[Dry-Run] Mise à jour des processus simulée.\n");
            mgr.running = false; // Quitter après le premier cycle
        }

        // 3 - Gestion clavier
        int key = ui_get_input();
        if (key == 'q') {
            mgr.running = false;
        }

        // 4 - Pause (seulement en mode normal)
        if (mgr.running) {
            sleep(1); // 1 seconde (on avait 500ms avec usleep)
        }
    }

    // Fin Nettoyage
    if (!mgr.dry_run) ui_clean();
    manager_clean(&mgr);

    return 0;
}