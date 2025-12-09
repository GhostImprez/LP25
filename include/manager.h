#ifndef MANAGER_H
#define MANAGER_H

#include <stdbool.h>
#include <sys/types.h>

// --- Énumérations ---

// Liste des états possibles d'un processus Linux
typedef enum {
    STATE_RUNNING = 'R',
    STATE_SLEEPING = 'S',
    STATE_ZOMBIE = 'Z',
    STATE_STOPPED = 'T',
    STATE_IDLE = 'I',
    STATE_WAKING = 'W',
    STATE_WAKEKILL = 'K',
    STATE_PARKED = 'P',
    STATE_DEAD = 'X',
    STATE_UNKNOWN = '?'
} process_state_e;

// Types de connexions réseau supportées
typedef enum {
    CONN_LOCAL,
    CONN_SSH,
    CONN_TELNET
} conn_type_e;

// --- Structures de Données ---

// Structure représentant un processus (compatible avec process.c)
typedef struct {
    pid_t pid;
    char user[32];
    char command[256];      // process.c utilise 'command', pas 'cmd'
    process_state_e state;
    
    double time_sec;        // Temps total CPU en secondes

    // --- Variables techniques pour le calcul CPU (Ne pas renommer) ---
    double cpu_usage;       // Pourcentage d'utilisation CPU
    long prev_proc_ticks;   // État précédent du processus (pour le delta)
    long prev_sys_ticks;    // État précédent du système (pour le delta)
    int has_prev;           // Est-ce qu'on a un historique pour calculer ?

    // --- Mémoire ---
    double mem_usage;       // Pourcentage de RAM utilisée
} process_t;

// Liste dynamique de processus
typedef struct {
    process_t *list;        // Le tableau des processus
    int count;              // Nombre actuel d'éléments
    int capacity;           // Taille réelle allouée en mémoire
} process_list_t;

// Structure représentant une machine (Locale ou Distante)
typedef struct {
    char *name;             // Nom affiché dans l'onglet
    char *host;             // Adresse IP
    int port;
    char *user;
    char *password;
    conn_type_e type;

    bool connected;
    
    // Contient la liste des processus de cette machine
    process_list_t processes; 
} machine_t;

// Contexte global de l'application
typedef struct {
    // Configuration
    bool dry_run;           // Mode sans interface graphique
    bool show_all;          // Afficher local + distants
    char *config_path;      // Chemin vers le fichier .config

    // Gestion des machines
    machine_t *machines;    // Tableau des machines
    int machine_count;      // Nombre de machines
    int machine_capacity;   // Capacité du tableau

    // État du programme
    bool running;           // Contrôle la boucle principale
    int current_machine;    // Onglet actuellement affiché
} manager_t;

// --- Prototypes ---
void manager_init(manager_t *m);
void manager_clean(manager_t *m);
int manager_add_machine(manager_t *m, const char *name, const char *host, int port, conn_type_e type);

#endif