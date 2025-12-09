#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>

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
    
    // Contexte UI
    ui_ctx_t ui_ctx;
    // Important : initialiser à 0
    memset(&ui_ctx, 0, sizeof(ui_ctx_t));

    // Parsing
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
            case 'a': mgr.show_all = true; break;
            case 'c': printf("[INFO] Config non implémentée pour MVP local\n"); break;
        }
    }

    // Ajout machine locale par défaut
    if (mgr.machine_count == 0) {
        manager_add_machine(&mgr, "Systeme local", "127.0.0.1", 0, CONN_LOCAL);
    }

    // Init UI
    if (!mgr.dry_run) {
        ui_init(&ui_ctx);
    }

    // Boucle Principale
    while (mgr.running) {
        // 1. UPDATE (Module Process)
        for(int i = 0; i < mgr.machine_count; i++) {
            if (mgr.machines[i].type == CONN_LOCAL) {
                // Fonction de P1 (process.c)
                update_local_processes(&mgr.machines[i]);
            }
        }

        // 2. DRAW (Module UI)
        if (!mgr.dry_run) {
            // Fonction de P3 (ui.c) adaptée
            ui_draw(&ui_ctx, mgr.machines, mgr.machine_count);
        } else {
            // Mode texte pour debug
            printf("[Dry-Run] Machine 0: %d processus\n", mgr.machines[0].processes.count);
            mgr.running = false; 
        }

        // 3. INPUT (Module UI)
        if (!mgr.dry_run) {
            // On passe le nombre de procs de la machine ACTIVE
            int current_count = mgr.machines[ui_ctx.tab].processes.count;
            
            int key = ui_input(&ui_ctx, mgr.machine_count, current_count);
            
            if (key == 'q') mgr.running = false;
            
            // TODO: Ajouter ici les appels à interaction_processus.c
            // Si key == KEY_F(5) -> pause_process(...)
        }

        // 4. TIMING
        if (mgr.running) usleep(100000); // 100ms
    }

    if (!mgr.dry_run) ui_end();
    manager_clean(&mgr);

    return 0;
}