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



void update_local_processes(machine_t *m){
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

        //on lit le Uid dans le fichier status
        int uid = -1;
        char line_usr[256];
        while (fgets(line_usr, sizeof(line_usr), f_user)) {
            if (strncmp(line_usr, "Uid:", 4) == 0) {
                sscanf(line_usr, "Uid:\t%d", &uid);  // récupérer le premier UID
                break;
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
                    default:  p->state = STATE_UNKNOWN; break;
                }
                break;  // on a trouvé l'état, plus besoin de continuer
            }
        }

        fclose(f_state);
        

        //Lecture de CPU%-----------------------------------------

        //Lecture de MEM%-----------------------------------------

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

    for (int i = 0; i < m->processes.count; i++) {
    process_t *p = &m->processes.list[i]; // pointeur vers le iᵉ processus

    // Exemple d'affichage
    printf("PID: %d\n", p->pid);
    printf("User: %s\n", p->user);
    printf("Command: %s\n", p->command);
    printf("State: %c\n", p->state);
    printf("TIME: %.2f sec\n", p->time_sec);
    printf("---\n");
}

    return 0;
    
}