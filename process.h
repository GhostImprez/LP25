#ifndef PROCESS_H
#define PROCESS_H

#include <stdbool.h>
#include <sys/types.h>

// --- Définitions pour le Module Process (Personne 1) et UI (Personne 3) ---

typedef enum {
    STATE_RUNNING = 'R',
    STATE_SLEEPING = 'S',
    STATE_ZOMBIE = 'Z',
    STATE_STOPPED = 'T',

    //ajout d'etats
    STATE_IDLE = 'I',
    STATE_WAKING = 'W',
    STATE_WAKEKILL = 'K',
    STATE_PARKED = 'P',
    STATE_DEAD = 'X',

    STATE_UNKNOWN = '?'
} process_state_e;


typedef struct {
    pid_t pid;
    char user[32];
    char command[256];
    process_state_e state;
    double time_sec;    // temps total CPU en secondes

    //partie %cpu----------------
    double cpu_usage;       // %
    long prev_proc_ticks;   // (utime+stime) précédente valeur
    long prev_sys_ticks;    // total CPU machine précédent
    int has_prev;           // 0 = première mesure, 1 = delta possible

    //partie %mem----------------
    double mem_usage;       // %
    
} process_t;


typedef struct {
    process_t *list;        // Tableau
    int count;              // Nombre actuel d'éléments
    int capacity;           // Taille allouée en mémoire
} process_list_t;

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

//prototypes des fonctions process.c
long read_total_cpu_ticks();
int read_process_ticks(pid_t pid, long *result);
void compute_cpu_usage(process_t *p);
process_t *find_process_by_pid(process_t *list, int count, pid_t pid);
void update_local_processes(machine_t *m);
long read_total_memory_kb();

#endif