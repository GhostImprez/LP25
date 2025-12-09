#include <stdio.h>
#include <string.h> 
#include <stdlib.h> 
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <ctype.h>
#include <pwd.h>
#include <stdbool.h> 
#include "process.h"

// --- Fonctions Utilitaires (Lecture des fichiers système) ---

// Lit le nombre total de "ticks" CPU depuis le démarrage de la machine
// Source : /proc/stat
long read_total_cpu_ticks() {
    FILE *f = fopen("/proc/stat", "r");
    if (!f) return 0;

    char buffer[256];
    // On lit la première ligne qui commence par "cpu"
    if (!fgets(buffer, sizeof(buffer), f)) {
        fclose(f);
        return 0;
    }
    fclose(f);

    // On récupère toutes les colonnes de temps (user, nice, system, idle...)
    long user, nicev, system, idle, iowait, irq, softirq, steal;
    sscanf(buffer, "cpu %ld %ld %ld %ld %ld %ld %ld %ld",
           &user, &nicev, &system, &idle, &iowait, &irq, &softirq, &steal);

    // La somme de tout donne le temps total écoulé
    return user + nicev + system + idle + iowait + irq + softirq + steal;
}

// Lit la quantité totale de RAM installée (en Ko)
// Source : /proc/meminfo
long read_total_memory_kb() {
    FILE *f = fopen("/proc/meminfo", "r");
    if (!f) return 0;

    char line[256];
    long mem_kb = 0;
    // On cherche la ligne "MemTotal:"
    while (fgets(line, sizeof(line), f)) {
        if (sscanf(line, "MemTotal:%ld kB", &mem_kb) == 1) {
            break;
        }
    }
    fclose(f);
    return mem_kb;
}

// Lit les ticks CPU consommés par un processus spécifique
// Source : /proc/[pid]/stat
int read_process_ticks(pid_t pid, long *result) {
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);
    
    FILE *f = fopen(path, "r");
    if (!f) return 0;

    // Le format de /proc/pid/stat est complexe car le nom du processus peut contenir des espaces
    // On lit PID, comm et state, puis on saute les champs inutiles
    int pid_tmp;
    char comm[256];
    char state;
    long utime, stime;

    fscanf(f, "%d %s %c", &pid_tmp, comm, &state);
    
    // On ignore les champs 4 à 13
    for (int i = 0; i < 10; i++) {
        char dummy[64];
        fscanf(f, "%s", dummy);
    }

    // On récupère utime (temps utilisateur) et stime (temps noyau)
    fscanf(f, "%ld %ld", &utime, &stime);
    fclose(f);

    *result = utime + stime;
    return 1;
}

// Cherche un processus dans une liste existante par son PID
process_t *find_process_by_pid(process_t *list, int count, pid_t pid) {
    for (int i = 0; i < count; i++) {
        if (list[i].pid == pid)
            return &list[i];
    }
    return NULL;
}

// Calcule le % CPU en comparant avec l'ancienne mesure
void compute_cpu_usage(process_t *p) {
    long proc_ticks;
    
    // Si on ne peut pas lire les infos (ex: processus fini entre temps), on quitte
    if (!read_process_ticks(p->pid, &proc_ticks))
        return; 

    long sys_ticks = read_total_cpu_ticks();

    // Si c'est la première fois qu'on voit ce processus, on ne peut pas calculer de Delta
    if (!p->has_prev) {
        p->prev_proc_ticks = proc_ticks;
        p->prev_sys_ticks = sys_ticks;
        p->has_prev = 1;
        p->cpu_usage = 0.0;
        return;
    }

    // Calcul des différences (Delta)
    long delta_proc = proc_ticks - p->prev_proc_ticks;
    long delta_sys  = sys_ticks  - p->prev_sys_ticks;

    // Règle de trois pour le pourcentage
    if (delta_sys > 0)
        p->cpu_usage = 100.0 * (double)delta_proc / (double)delta_sys;
    else
        p->cpu_usage = 0.0;

    // Sauvegarde pour le prochain tour
    p->prev_proc_ticks = proc_ticks;
    p->prev_sys_ticks = sys_ticks;
}


// --- Fonction Principale du Module ---

void update_local_processes(machine_t *m) {
    // 1. Sauvegarde de l'ancienne liste pour les calculs CPU (Delta)
    process_t *old_list = m->processes.list;
    int old_count = m->processes.count;

    // 2. Préparation de la nouvelle liste
    // On alloue une capacité initiale arbitraire (ex: 64 processus)
    m->processes.capacity = 64; 
    m->processes.list = malloc(sizeof(process_t) * m->processes.capacity);
    if (!m->processes.list) exit(EXIT_FAILURE);

    int n_process = 0;
    long mem_total_kb = read_total_memory_kb();

    // 3. Parcours du dossier /proc
    DIR *proc = opendir("/proc");
    if (!proc) {
        perror("Impossible d'ouvrir /proc");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(proc)) != NULL) {
        // On ne s'intéresse qu'aux dossiers qui sont des nombres (les PIDs)
        if (!isdigit(entry->d_name[0])) continue;

        // Gestion de la mémoire dynamique : Agrandissement si nécessaire
        if (n_process >= m->processes.capacity) {
            int new_cap = m->processes.capacity * 2;
            process_t *tmp = realloc(m->processes.list, sizeof(process_t) * new_cap);
            if (!tmp) exit(EXIT_FAILURE);
            m->processes.list = tmp;
            m->processes.capacity = new_cap;
        }

        // Pointeur vers le processus en cours de remplissage
        process_t *p = &m->processes.list[n_process];

        // --- A. Récupération du PID ---
        p->pid = atoi(entry->d_name);

        // --- B. Récupération Utilisateur et Mémoire RSS ---
        char path_status[64];
        snprintf(path_status, sizeof(path_status), "/proc/%d/status", p->pid);
        
        FILE *f_status = fopen(path_status, "r");
        long rss_kb = 0;
        int uid = -1;
        
        if (f_status) {
            char line[256];
            while (fgets(line, sizeof(line), f_status)) {
                if (strncmp(line, "Uid:", 4) == 0) {
                    sscanf(line, "Uid:\t%d", &uid);
                }
                else if (strncmp(line, "VmRSS:", 6) == 0) {
                    sscanf(line, "VmRSS:%ld kB", &rss_kb);
                }
                else if (strncmp(line, "State:", 6) == 0) {
                    // --- C. Récupération de l'État ---
                    char state_char;
                    sscanf(line, "State:\t%c", &state_char);
                    switch (state_char) {
                        case 'R': p->state = STATE_RUNNING; break;
                        case 'S': p->state = STATE_SLEEPING; break;
                        case 'Z': p->state = STATE_ZOMBIE; break;
                        case 'T': p->state = STATE_STOPPED; break;
                        case 'I': p->state = STATE_IDLE; break;
                        case 'X': p->state = STATE_DEAD; break;
                        default:  p->state = STATE_UNKNOWN; break;
                    }
                }
            }
            fclose(f_status);
        } else {
            // Le processus a peut-être disparu entre temps
            continue; 
        }

        // Conversion UID -> Nom d'utilisateur
        struct passwd *pw = getpwuid(uid);
        if (pw) {
            strncpy(p->user, pw->pw_name, sizeof(p->user) - 1);
            p->user[sizeof(p->user) - 1] = '\0';
        } else {
            snprintf(p->user, sizeof(p->user), "%d", uid);
        }

        // --- D. Calcul utilisation Mémoire % ---
        if (mem_total_kb > 0 && rss_kb > 0) {
            p->mem_usage = 100.0 * (double)rss_kb / (double)mem_total_kb;
        } else {
            p->mem_usage = 0.0;
        }

        // --- E. Calcul CPU % ---
        // On essaye de retrouver ce processus dans l'ancienne liste
        process_t *old = find_process_by_pid(old_list, old_count, p->pid);

        if (old) {
            // On reprend l'historique pour calculer le delta
            p->prev_proc_ticks = old->prev_proc_ticks;
            p->prev_sys_ticks  = old->prev_sys_ticks;
            p->has_prev        = old->has_prev;
            p->cpu_usage       = old->cpu_usage; // Garde l'ancienne valeur en attendant le refresh
        } else {
            // Nouveau processus
            p->prev_proc_ticks = 0;
            p->prev_sys_ticks  = 0;
            p->has_prev        = 0;
            p->cpu_usage       = 0.0;
        }
        
        // Mise à jour du calcul
        compute_cpu_usage(p);

        // --- F. Récupération de la Commande ---
        char cmd_path[64];
        snprintf(cmd_path, sizeof(cmd_path), "/proc/%d/cmdline", p->pid);
        FILE *f_cmd = fopen(cmd_path, "r");
        if (f_cmd) {
            size_t len = fread(p->command, 1, sizeof(p->command) - 1, f_cmd);
            fclose(f_cmd);
            
            if (len > 0) {
                p->command[len] = '\0';
                // Cmdline sépare les arguments par \0, on les remplace par des espaces
                for (size_t i = 0; i < len; i++) {
                    if (p->command[i] == '\0') p->command[i] = ' ';
                }
            } else {
                strcpy(p->command, "unknown");
            }
        } else {
            strcpy(p->command, "unknown");
        }

        n_process++;
    }

    closedir(proc);

    // 4. Nettoyage et Finalisation
    if (old_list) {
        free(old_list); // On libère l'ancienne liste
    }
    
    m->processes.count = n_process;
}