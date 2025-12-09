#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <time.h>

// Inclusion des modules réels
#include "manager.h"
#include "process.h" // Module P1
#include "ui.h"      // Module P3

static struct option options_longues[] = {
    {"help", no_argument, 0, 'h'},
    {"dry-run", no_argument, 0, 0},   
    {"remote-config", required_argument, 0, 'c'},
    {"connexion-type", required_argument, 0, 't'},
    {"port", required_argument, 0, 'P'},
    {"login", required_argument, 0, 'l'},
    {"remote-server", required_argument, 0, 's'},
    {"username", required_argument, 0, 'u'},
    {"password", required_argument, 0, 'p'},
    {"all", no_argument, 0, 'a'},
    {0, 0, 0, 0}
};

int main(int argc, char *argv[])
{
    manager_t mgr;
    manager_init(&mgr);
    
    // Contexte UI (pour savoir quelle ligne est sélectionnée, scroll, onglet...)
    ui_ctx_t ui_ctx;
    memset(&ui_ctx, 0, sizeof(ui_ctx_t)); 

    // --- Parsing ---
    int opt, idx;
    while ((opt = getopt_long(argc, argv, "hc:l:a", options_longues, &idx)) != -1) {
        switch (opt) {
            case 0: 
                if (strcmp(options_longues[idx].name, "dry-run") == 0) mgr.dry_run = true;
                break;
            case 'h':
                printf("Usage: %s [OPTIONS]\n", argv[0]);
                manager_clean(&mgr);
                return 0;
            case 'c':
                printf("[INFO] Config non chargée pour le MVP Local.\n");
                break;
            case 'a':
                mgr.show_all = true;
                break;
        }
    }

    // --- Initialisation ---
    if (mgr.machine_count == 0) {
        manager_add_machine(&mgr, "Systeme Local", "127.0.0.1", 0, CONN_LOCAL);
    }

    if (!mgr.dry_run) {
        ui_init(&ui_ctx);
    }

    // --- Boucle Principale ---
    while (mgr.running) {
        
        // A. Mise à jour des données 
        for(int i = 0; i < mgr.machine_count; i++) {
            if (mgr.machines[i].type == CONN_LOCAL) {
                update_local_processes(&mgr.machines[i]);
            }
        }

        // B. Affichage 
        if (!mgr.dry_run) {
            ui_draw(&ui_ctx, mgr.machines, mgr.machine_count);
        } else {
            printf("[Dry-Run] Machine '%s' : %d processus actifs.\n", 
                   mgr.machines[0].name, mgr.machines[0].processes.count);
            mgr.running = false; // Arrêt immédiat en mode test
        }

        // C. Gestion des Entrées Clavier
        if (!mgr.dry_run) {
            int current_count = mgr.machines[ui_ctx.tab].processes.count;
            
            // Lecture clavier
            int key = ui_input(&ui_ctx, mgr.machine_count, current_count);
            
            // --- Logique des touches ---
            if (key == 'q') {
                mgr.running = false;
            }
            
            // Touche F1 : Aide
            else if (key == KEY_F(1)) {
                aide_process(); 
            }

            // Touches d'action (F5 à F8)
            else if (key >= KEY_F(5) && key <= KEY_F(8)) {
                
                // 1. On identifie la machine active (Onglet)
                machine_t *curr_machine = &mgr.machines[ui_ctx.tab];

                // 2. On vérifie qu'on ne tape pas dans le vide (sélection valide)
                if (curr_machine->processes.count > 0 && ui_ctx.sel < curr_machine->processes.count) {
                    
                    // 3. On récupère le PID de la ligne surlignée
                    pid_t target_pid = curr_machine->processes.list[ui_ctx.sel].pid;

                    // 4. On appelle la bonne fonction d'interaction
                    switch (key) {
                        case KEY_F(5): // Pause
                            pause_process(target_pid);
                            break;
                        case KEY_F(6): // Stop (SIGTERM)
                            arret_process(target_pid);
                            break;
                        case KEY_F(7): // Kill (SIGKILL)
                            tuer_process(target_pid);
                            break;
                        case KEY_F(8): // Redémarrer / Reprendre (SIGCONT)
                            redemarrer_process(target_pid);
                            break;
                    }
                }
            }
        }

        // D. Temporisation (Remplacement moderne de usleep)
        if (mgr.running) {
            struct timespec ts;
            ts.tv_sec = 0;   
            ts.tv_nsec = 100000000;     // 100 000 000 ns = 100ms 
            nanosleep(&ts, NULL);
        }
    }

    // --- Nettoyage ---
    if (!mgr.dry_run) ui_end();
    manager_clean(&mgr);

    return 0;
}