#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <time.h>

// Inclusion des modules réels
#include "manager.h"
#include "process.h"
#include "ui.h"
#include "network.h"

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
    memset(&ui_ctx, 0, sizeof(ui_ctx_t)); 

    // --- Variables pour parser les options ---
    char *config_file = NULL;
    char *login_str = NULL;
    char *username_str = NULL;
    char *remote_server = NULL;
    int port = -1;
    bool all_flag = false;

    // --- Parsing ---
    int opt, idx;
    while ((opt = getopt_long(argc, argv, "hc:l:u:s:P:a", options_longues, &idx)) != -1) {
        switch (opt) {
            case 0: 
                if (strcmp(options_longues[idx].name, "dry-run") == 0) 
                    mgr.dry_run = true;
                break;
            case 'h':
                printf("Usage: %s [OPTIONS]\n", argv[0]);
                printf("Options:\n");
                printf("  --help, -h              Afficher l'aide\n");
                printf("  --all, -a               Afficher toutes les machines\n");
                printf("  --remote-config, -c     Fichier de configuration\n");
                printf("  --login, -l             Login (user@host)\n");
                printf("  --username, -u          Nom d'utilisateur\n");
                printf("  --remote-server, -s     Serveur distant\n");
                printf("  --port, -P              Port SSH\n");
                manager_clean(&mgr);
                return 0;
            case 'c':
                config_file = optarg;
                break;
            case 'l':
                login_str = optarg;
                break;
            case 'u':
                username_str = optarg;
                break;
            case 's':
                remote_server = optarg;
                break;
            case 'P':
                port = atoi(optarg);
                break;
            case 'a':
                all_flag = true;
                break;
            default:
                fprintf(stderr, "Options invalides. Utilisez --help pour l'aide.\n");
                manager_clean(&mgr);
                return 1;
        }
    }

    // --- Détermination du cas et initialisation des machines ---
    
    // CAS 4: --all + --remote-config
    if (all_flag && config_file) {
        lecture_fichier_config(config_file, &mgr);
        for (int i = 0; i < mgr.machine_count; i++) {
            if (mgr.machines[i].type == CONN_SSH) {
                // start persistent master (may prompt once)
                ssh_start_master(&mgr.machines[i]);
            }
            update_remote_processes(&mgr.machines[i]);
        }
    }
    // CAS 3: --login + (--port)
    else if (login_str && !username_str && !remote_server) {
        machine_t m;
        memset(&m, 0, sizeof(machine_t));
        m.type = CONN_SSH;
        
        if (parse_login(login_str, port, &m) != 0) {
            fprintf(stderr, "Erreur: format --login invalide (attendu: user@host)\n");
            manager_clean(&mgr);
            return 1;
        }
        
        manager_add_machine(&mgr, "Machine Distante", m.host, m.port, CONN_SSH, m.user, NULL);
        free(m.user);
        free(m.host);
        // start ssh master for this machine (may prompt once)
        ssh_start_master(&mgr.machines[0]);
        update_remote_processes(&mgr.machines[0]);
    }
    // CAS 2: --username + --remote-server + (--port)
    else if (username_str && remote_server && !login_str) {
        machine_t m;
        memset(&m, 0, sizeof(machine_t));
        m.type = CONN_SSH;
        
        if (parse_username_host(username_str, remote_server, port, &m) != 0) {
            fprintf(stderr, "Erreur: impossible de configurer la machine distante\n");
            manager_clean(&mgr);
            return 1;
        }
        
        manager_add_machine(&mgr, "Machine Distante", m.host, m.port, CONN_SSH, m.user, NULL);
        free(m.user);
        free(m.host);
        // start ssh master for this machine (may prompt once)
        ssh_start_master(&mgr.machines[0]);
        update_remote_processes(&mgr.machines[0]);
    }
    // CAS 1: AUCUNE OPTION (machine locale)
    else if (!login_str && !username_str && !remote_server && !config_file && !all_flag) {
        manager_add_machine(&mgr, "Systeme Local", "127.0.0.1", 0, CONN_LOCAL, NULL, NULL);
        update_local_processes(&mgr.machines[0]);
    }
    // OPTIONS INVALIDES
    else {
        fprintf(stderr, "Erreur: combinaison d'options invalide.\n");
        fprintf(stderr, "Utilisations valides:\n");
        fprintf(stderr, "  1. Aucune option (machine locale)\n");
        fprintf(stderr, "  2. --username + --remote-server [--port]\n");
        fprintf(stderr, "  3. --login [--port]\n");
        fprintf(stderr, "  4. --all + --remote-config\n");
        manager_clean(&mgr);
        return 1;
    }

    // Vérifier qu'on a au moins une machine
    if (mgr.machine_count == 0) {
        fprintf(stderr, "Erreur: aucune machine configurée\n");
        manager_clean(&mgr);
        return 1;
    }

    // --- Initialisation UI ---
    if (!mgr.dry_run) {
        ui_init(&ui_ctx);
    }

    // --- Boucle Principale ---
    while (mgr.running) {
        
        // A. Mise à jour des données 
        for(int i = 0; i < mgr.machine_count; i++) {
            if (mgr.machines[i].type == CONN_LOCAL) {
                update_local_processes(&mgr.machines[i]);
            } else {
                update_remote_processes(&mgr.machines[i]);
            }
        }

        // B. Affichage 
        if (!mgr.dry_run) {
            ui_draw(&ui_ctx, mgr.machines, mgr.machine_count);
        } else {
            for(int i = 0; i < mgr.machine_count; i++) {
                printf("[Dry-Run] Machine '%s' : %d processus actifs.\n", 
                       mgr.machines[i].name, mgr.machines[i].processes.count);
            }
            mgr.running = false;
        }

        // C. Gestion des Entrées Clavier
        if (!mgr.dry_run) {
            int current_count = mgr.machines[ui_ctx.tab].processes.count;
            int key = ui_input(&ui_ctx, mgr.machine_count, current_count);
            
            if (key == 'q') {
                mgr.running = false;
            }
            else if (key == KEY_F(1)) {
                aide_process(); 
            }
            else if (key >= KEY_F(5) && key <= KEY_F(8)) {
                machine_t *curr_machine = &mgr.machines[ui_ctx.tab];
                
                // Vérifier que c'est une machine locale avant d'agir
                if (curr_machine->type != CONN_LOCAL) {
                    // Afficher un message d'erreur ou ignorer
                    continue;
                }
                
                if (curr_machine->processes.count > 0 && ui_ctx.sel < curr_machine->processes.count) {
                    pid_t target_pid = curr_machine->processes.list[ui_ctx.sel].pid;
                    
                    switch (key) {
                        case KEY_F(5): pause_process(target_pid); break;
                        case KEY_F(6): arret_process(target_pid); break;
                        case KEY_F(7): tuer_process(target_pid); break;
                        case KEY_F(8): redemarrer_process(target_pid); break;
                    }
                }
            }
        }

        // D. Temporisation
        if (mgr.running) {
            struct timespec ts;
            ts.tv_sec = 0;   
            ts.tv_nsec = 100000000;
            nanosleep(&ts, NULL);
        }
    }

    // --- Nettoyage ---
    if (!mgr.dry_run) ui_end();
    // Stop SSH masters
    for (int i = 0; i < mgr.machine_count; i++) {
        if (mgr.machines[i].type == CONN_SSH) {
            ssh_stop_master(&mgr.machines[i]);
        }
    }
    manager_clean(&mgr);

    return 0;
}