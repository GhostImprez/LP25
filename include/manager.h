#ifndef MANAGER_H
#define MANAGER_H

#include <stdbool.h>
#include <sys/types.h>

// --- Définitions pour le Module Process (Personne 1) et UI (Personne 3) ---

typedef enum {
    STATE_RUNNING = 'R',
    STATE_SLEEPING = 'S',
    STATE_ZOMBIE = 'Z',
    STATE_STOPPED = 'T',
    STATE_UNKNOWN = '?'
} process_state_e;


typedef struct {
    pid_t pid;
    char user[32];
    char command[256];
    process_state_e state;
    double cpu_usage;       // %
    double mem_usage;       // %
} process_t;


typedef struct {
    process_t *list;        // Tableau
    int count;              // Nombre actuel d'éléments
    int capacity;           // Taille allouée en mémoire
} process_list_t;

// Définition manager

typedef enum {
    CONN_LOCAL,
    CONN_SSH,
    CONN_TELNET
} conn_type_e;


typedef struct {
    char *name;             // Nom (ex: "Local")
    char *host;             // IP ou nom de domaine
    int port;
    char *user;
    char *password;
    conn_type_e type;

    bool connected;
    
    // Les processus de CETTE machine
    process_list_t processes; 
} machine_t;


typedef struct {
    // Config
    bool dry_run;
    bool show_all;
    char *config_path;

    // Données
    machine_t *machines;    // Tableau des machines surveillées
    int machine_count;
    int machine_capacity;

    // Etat
    bool running;           // Contrôle la boucle principale
    int current_machine;    // Index de l'onglet actif
} manager_t;

// Prototypes pour test main.c

void manager_init(manager_t *m);
void manager_clean(manager_t *m);
int manager_add_machine(manager_t *m, const char *name, const char *host, int port, conn_type_e type);

#endif