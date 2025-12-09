#include <stdio.h>
#include <string.h> 
#include <stdlib.h> 
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <ctype.h>
#include <pwd.h>
#include <stdbool.h> 
#include "manager.h"

/*
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



LISTE DES INFORMATIONS A AFFICHER POUR CHAQUE PROCESSUS :

PID - OK
USER - OK
PRI - ?
NI - ?
VIRT - ?
RES - ?
SHR - ?
S - OK
CPU% - OK
MEM% - OK
TIME+ - ?
COMMAND - OK

il manque :
- PRI (priorité)
- NI (nice value)
- VIRT (taille mémoire virtuelle)
- RES (taille mémoire résidente)
- SHR (taille mémoire partagée)
- TIME+ (temps CPU total utilisé par le processus)

Informations retenues :
- PID
- USER
- S (state)
- CPU%
- MEM%
- TIME+ -----------
- COMMAND

*/

/*
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


*/

// Fonction pour lire le total des ticks CPU de la machine depuis /proc/stat

long read_total_cpu_ticks() {
    FILE *f = fopen("/proc/stat", "r");
    if (!f) return 0;

    char buffer[256];
    fgets(buffer, sizeof(buffer), f); // lit la ligne "cpu  ..."

    fclose(f);

    // on lit tous les champs sauf "cpu"
    long user, nicev, system, idle, iowait, irq, softirq, steal;
    sscanf(buffer, "cpu %ld %ld %ld %ld %ld %ld %ld %ld",
           &user, &nicev, &system, &idle, &iowait, &irq, &softirq, &steal);

    return user + nicev + system + idle + iowait + irq + softirq + steal;
}

// Lire la mémoire totale (kB) depuis /proc/meminfo
long read_total_memory_kb() {
    FILE *f = fopen("/proc/meminfo", "r");
    if (!f) return 0;

    char line[256];
    long mem_kb = 0;
    while (fgets(line, sizeof(line), f)) {
        if (sscanf(line, "MemTotal:%ld kB", &mem_kb) == 1) {
            break;
        }
    }
    fclose(f);
    return mem_kb;
}

// Fonction pour lire les ticks CPU d'un processus depuis /proc/PID/stat

int read_process_ticks(pid_t pid, long *result) {
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);
    FILE *f = fopen(path, "r");
    if (!f) return 0;

    int pid_tmp;
    char comm[256];
    char state;
    long utime, stime;

    // sauter les champs 1 à 13
    fscanf(f, "%d %s %c", &pid_tmp, comm, &state);
    // après avoir lu pid, comm et state, il faut ignorer les champs 4..13 (10 champs)
    for (int i = 0; i < 10; i++) {
        fscanf(f, "%*s");
    }

    // champ 14 = utime, champ 15 = stime
    fscanf(f, "%ld %ld", &utime, &stime);

    fclose(f);

    *result = utime + stime;
    return 1;
}

//Fonction pour calculer l'utilisation CPU d'un processus en %

void compute_cpu_usage(process_t *p) {
    long proc_ticks;
    if (!read_process_ticks(p->pid, &proc_ticks))
        return; // process disparu

    long sys_ticks = read_total_cpu_ticks();

    // Première mesure → on ne peut PAS calculer un delta
    if (!p->has_prev) {
        p->prev_proc_ticks = proc_ticks;
        p->prev_sys_ticks = sys_ticks;
        p->has_prev = 1;
        p->cpu_usage = 0.0;
        return;
    }

    long delta_proc = proc_ticks - p->prev_proc_ticks;
    long delta_sys  = sys_ticks  - p->prev_sys_ticks;

    if (delta_sys > 0)
        p->cpu_usage = 100.0 * (double)delta_proc / (double)delta_sys;
    else
        p->cpu_usage = 0.0;

    // mettre à jour les anciennes valeurs
    p->prev_proc_ticks = proc_ticks;
    p->prev_sys_ticks = sys_ticks;
}

process_t *find_process_by_pid(process_t *list, int count, pid_t pid) {
    for (int i = 0; i < count; i++) {
        if (list[i].pid == pid)
            return &list[i];
    }
    return NULL;
}

void update_local_processes(machine_t *m){

    // Avant la boucle sur /proc, on sauvegarde l'ancienne liste :
    process_t *old_list = m->processes.list;
    int old_count = m->processes.count;

    if (m->processes.count == 0) {
        m->processes.capacity = 64; // capacité initiale arbitraire
        m->processes.list = malloc(sizeof(process_t) * m->processes.capacity);
        if (!m->processes.list) exit(1);
    }

    int n_process = 0;

    DIR *proc = opendir("/proc");
    struct dirent *entry;
    while ((entry = readdir(proc)) != NULL){
        if (!isdigit(entry->d_name[0])) continue;

        if (n_process >= m->processes.capacity) {
            // réallocation si on dépasse la capacité
            int new_cap = m->processes.capacity * 2;
            process_t *tmp = realloc(m->processes.list, sizeof(process_t) * new_cap);
            if (!tmp) exit(1);
            m->processes.list = tmp;
            m->processes.capacity = new_cap;
        }

        // remplir process_t
        process_t *p = &m->processes.list[n_process];

        //Lecture du pid-----------------------------------------
        p->pid = atoi(entry->d_name);

        //Lecture de USER----------------------------------------
        //on ouvre le fichier /proc/PID/status
        char path_usr[64];
        snprintf(path_usr, sizeof(path_usr), "/proc/%d/status", p->pid);
        FILE *f_user = fopen(path_usr, "r");
        if (!f_user) continue;  // processus peut avoir disparu

        //on lit le Uid et VmRSS dans le fichier status
        int uid = -1;
        long rss_kb = -1;
        char line_usr[256];
        while (fgets(line_usr, sizeof(line_usr), f_user)) {
            if (strncmp(line_usr, "Uid:", 4) == 0) {
                sscanf(line_usr, "Uid:\t%d", &uid);  // récupérer le premier UID
            }
            else if (strncmp(line_usr, "VmRSS:", 6) == 0) {
                // format: VmRSS:\t   1234 kB
                sscanf(line_usr, "VmRSS:%ld kB", &rss_kb);
            }
        }
        fclose(f_user);

        //on convertit l'uid en nom d'utilisateur
        struct passwd *pw = getpwuid(uid);
        if (pw) {
            strncpy(p->user, pw->pw_name, sizeof(p->user));
            p->user[sizeof(p->user) - 1] = '\0'; // assurer la terminaison nulle
        } else {
            snprintf(p->user, sizeof(p->user), "%d", uid);
        }

        //Lecture de S (state)----------------------------------------
        //on ouvre le fichier /proc/PID/status
        char path_state[64];
        snprintf(path_state, sizeof(path_state), "/proc/%d/status", p->pid);
        FILE *f_state = fopen(path_state, "r");
        if (!f_state) continue;  // processus peut avoir disparu

        p->state = STATE_UNKNOWN;
        char line_state[256];
        while (fgets(line_state, sizeof(line_state), f_state)) {
            if (strncmp(line_state, "State:", 6) == 0) {
                char state_char;
                sscanf(line_state, "State:\t%c", &state_char);  // extraire le premier caractère
                switch (state_char) {
                    case 'R': p->state = STATE_RUNNING; break;
                    case 'S': p->state = STATE_SLEEPING; break;
                    case 'Z': p->state = STATE_ZOMBIE; break;
                    case 'T': p->state = STATE_STOPPED; break;

                    //ajout d'etats:
                    case 'I': p->state = STATE_IDLE; break;
                    case 'K': p->state = STATE_WAKEKILL; break;
                    case 'P': p->state = STATE_PARKED; break;
                    case 'X': p->state = STATE_DEAD; break;
                    case 'W': p->state = STATE_WAKING; break;
                    default:  p->state = STATE_UNKNOWN; break;
                }
                break;  // on a trouvé l'état, plus besoin de continuer
            }
        }

        fclose(f_state);
        

        //Lecture de CPU%-----------------------------------------
        // récupération des anciennes valeurs CPU ——
        process_t *old = find_process_by_pid(old_list, old_count, p->pid);

        if (old) {
            p->prev_proc_ticks = old->prev_proc_ticks;
            p->prev_sys_ticks  = old->prev_sys_ticks;
            p->has_prev        = old->has_prev;
            p->cpu_usage       = old->cpu_usage;
        } else {
            p->prev_proc_ticks = 0;
            p->prev_sys_ticks  = 0;
            p->has_prev        = 0;
            p->cpu_usage       = 0.0;
        }

        compute_cpu_usage(p);

        // Calcul de mem_usage en pourcentage (utilise VmRSS en kB)
        long mem_total_kb = read_total_memory_kb();
        if (mem_total_kb > 0 && rss_kb > 0) {
            p->mem_usage = 100.0 * (double)rss_kb / (double)mem_total_kb;
        } else {
            p->mem_usage = 0.0;
        }

        //Lecture de TIME+
        char stat_path[64];
        snprintf(stat_path, sizeof(stat_path), "/proc/%d/stat", p->pid);
        FILE *f_time = fopen(stat_path, "r");
        if (f_time) {
            int pid_tmp;
            char comm_tmp[256], state_tmp;
            long utime, stime;

            // Lire les trois premiers champs (PID, comm, state)
            fscanf(f_time, "%d %s %c", &pid_tmp, comm_tmp, &state_tmp);

            // Ignorer les champs 4 à 13
            for (int i = 0; i < 10; i++) {
                long dummy;
                fscanf(f_time, "%ld", &dummy);
            }

            // Lire utime (14e champ) et stime (15e champ)
            fscanf(f_time, "%ld %ld", &utime, &stime);
            fclose(f_time);

            // Stocker le temps total CPU en ticks
            long time_ticks = utime + stime;

            // Conversion en secondes (optionnel)
            long hz = sysconf(_SC_CLK_TCK); // ticks par seconde
            p->time_sec = (double)time_ticks / hz;
        } else {
            p->time_sec = 0.0;
        }

        //Lecture de COMMAND-----------------------------------------
        char cmd_path[64];
        snprintf(cmd_path, sizeof(cmd_path), "/proc/%d/cmdline", p->pid);
        FILE *f_cmd = fopen(cmd_path, "r");
        if (f_cmd) {
            size_t len = fread(p->command, 1, sizeof(p->command)-1, f_cmd);
            fclose(f_cmd);

            // remplacer les '\0' par des espaces pour lisibilité
            for (size_t i = 0; i < len; i++) {
                if (p->command[i] == '\0') p->command[i] = ' ';
            }

            p->command[len] = '\0'; // terminer la chaîne
        } else {
            snprintf(p->command, sizeof(p->command), "unknown");
        }

        //incrémentation du nombre de processus
         n_process++;

    }
    free(old_list);
    closedir(proc);

    m->processes.count = n_process;  // nombre réel de processus

}



int main() {

    machine_t *m = malloc(sizeof(machine_t));
    if (!m) return 1;
    m->processes.count = 0;
    m->processes.capacity = 0;
    m->processes.list = NULL;

    update_local_processes(m);


    /*
    Pour tester la lecture des processus locaux:
    mettre en commentaire le update_local_processes(m); ci-dessus
    et décommenter le code ci-dessous.
    */

    /*
    update_local_processes(m);
    usleep(500000);   // 500 ms
    update_local_processes(m);

    // --- Affichage ---
    for (int i = 0; i < m->processes.count; i++) {
    process_t *p = &m->processes.list[i]; // pointeur vers le iᵉ processus

    // affichage test
    printf("PID: %d\n", p->pid);
    printf("User: %s\n", p->user);
    printf("Command: %s\n", p->command);
    printf("State: %c\n", p->state);
    printf("TIME: %.2f sec\n", p->time_sec);
    printf("CPU%%: %.2f %%\n", p->cpu_usage);
    printf("MEM%%: %.2f %%\n", p->mem_usage);
    printf("---\n");
    }
    */

    

    return 0;
    
}