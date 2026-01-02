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
#include "manager.h"

/*
//La fonction suivante lit le fichier .config permettant d'initialiser une machine distante
les informations sont formalisées de cette façon :
nom_serveur1:adresse_serveur:port:username:password:type_connexion1
*/

void lecture_fichier_config(const char *chemin, manager_t *mgr) {
    FILE *f = fopen(chemin, "r");
    if (!f) {
        perror("fopen config");
        return;
    }

    char ligne[512];
    while (fgets(ligne, sizeof(ligne), f)) {
        char nom[128], adresse[128], username[64], password[64], type_str[16];
        int port;
        if (sscanf(ligne, "%127[^:]:%127[^:]:%d:%63[^:]:%63[^:]:%15s", 
                   nom, adresse, &port, username, password, type_str) == 6) {
            
            conn_type_e type;
            if (strcmp(type_str, "ssh") == 0) {
                type = CONN_SSH;
            } else if (strcmp(type_str, "telnet") == 0) {
                type = CONN_TELNET;
            } else {
                printf("Type de connexion inconnu pour la machine %s. Utilisation de SSH par défaut.\n", nom);
                type = CONN_SSH;
            }

            manager_add_machine(mgr, nom, adresse, port, type, username, password);
        }
    }

    fclose(f);
}

/*
La fonction ci dessous permet de récupérer les informations transmises via l'option --login
une chaine de caractère est transmise sous la forme :
username@adresse_serveur
la fonction parse_login décompose cette chaine et remplit les champs user et host de la machine passée en paramètre
*/

int parse_login(const char *login_str, int port, machine_t *m) {
    if (!login_str || !m) {
        return -1;
    }

    const char *at_sign = strchr(login_str, '@');
    if (!at_sign) {
        return -1;
    }

    size_t user_len = at_sign - login_str;
    m->user = malloc(user_len + 1);
    if (!m->user) {
        return -1;
    }
    strncpy(m->user, login_str, user_len);
    m->user[user_len] = '\0';

    m->host = strdup(at_sign + 1);
    if (!m->host) {
        free(m->user);
        return -1;
    }

    //si port non spécifié, on met 22 par défaut
    if (port <= 0) {
        m->port = 22;
    } else {
        m->port = port;
    }

    return 0;
}

/*
La fonction ci dessous permet de récupérer les informations transmises via les option --username et --remote-server
2 chaines de caractères sont transmises sous la forme :
username
adresse_serveur
la fonction récupère les chaines de caractères et remplit les champs user, host et port de la machine passée en paramètre
*/
int parse_username_host(const char *username_str, const char *host_str, int port, machine_t *m) {
    if (!username_str || !host_str || !m) {
        return -1;
    }

    m->user = strdup(username_str);
    if (!m->user) {
        return -1;
    }
    //si port non spécifié, on met 22 par défaut
    if (port <= 0) {
        m->port = 22;
    } else {
        m->port = port;
    }

    m->host = strdup(host_str);
    if (!m->host) {
        free(m->user);
        return -1;
    }

    return 0;
}



/*
La fonction ci dessous permet d'actualiser les processus d'une machine distance via ssh 
en executant la commande 'ps aux' et en enregistrant les informations dans la structure machine_t
*/
int update_remote_processes(machine_t *m) {
    if (!m || !m->host || !m->user) {
        return -1;
    }

    // Commande SSH
    char cmd[512];
    snprintf(cmd, sizeof(cmd),
             "ssh -p %d %s@%s \"ps aux\"",
             m->port > 0 ? m->port : 22,
             m->user,
             m->host);

    FILE *fp = popen(cmd, "r");
    if (!fp) {
        perror("popen ssh");
        m->connected = false;
        return -1;
    }

    m->connected = true;

    // Réinitialiser la liste des processus
    m->processes.count = 0;

    char line[1024];

    // Ignorer l'en-tête
    fgets(line, sizeof(line), fp);

    while (fgets(line, sizeof(line), fp)) {
        process_t p;
        memset(&p, 0, sizeof(process_t));

        /*
         * ps aux format simplifié :
         * USER PID %CPU %MEM VSZ RSS TTY STAT START TIME COMMAND
         */

        char stat[8];
        double cpu, mem;

        int matched = sscanf(line,
            "%31s %d %lf %lf %*d %*d %*s %7s %*s %*s %255[^\n]",
            p.user,
            &p.pid,
            &cpu,
            &mem,
            stat,
            p.command
        );

        if (matched < 6)
            continue;

        // Remplir les champs
        p.cpu_usage = cpu;
        p.mem_usage = mem;
        p.has_prev = 0;

        // État du processus (simplifié)
        switch (stat[0]) {
            case 'R': p.state = STATE_RUNNING; break;
            case 'S': p.state = STATE_SLEEPING; break;
            case 'Z': p.state = STATE_ZOMBIE; break;
            case 'T': p.state = STATE_STOPPED; break;
            default:  p.state = STATE_UNKNOWN;
        }

        // Agrandir le tableau si nécessaire
        if (m->processes.count >= m->processes.capacity) {
            m->processes.capacity = m->processes.capacity == 0 ? 64 : m->processes.capacity * 2;
            m->processes.list = realloc(
                m->processes.list,
                m->processes.capacity * sizeof(process_t)
            );
            if (!m->processes.list) {
                pclose(fp);
                return -1;
            }
        }

        m->processes.list[m->processes.count++] = p;
    }

    pclose(fp);
    return 0;
}

int main(){
    /*
    manager_t mgr;
    manager_init(&mgr);

    
    lecture_fichier_config(".config.txt", &mgr);
    machine_t *m = &mgr.machines[0]; // On prend la première machine distante ajoutée
    printf("nom: %s\n", m->name);
    printf("host: %s\n", m->host);
    printf("port: %d\n", m->port);
    printf("user: %s\n", m->user);
    printf("password: %s\n", m->password);
    printf("type: %d\n", m->type);
    
    //Mise à jour des processus distants
    if (update_remote_processes(m) == 0) {
        printf("Processus sur la machine distante %s:\n", m->name);
        for (int i = 0; i < m->processes.count; i++) {
            process_t *p = &m->processes.list[i];
            printf("PID: %d, User: %s, CPU%%: %.2f%%, MEM%%: %.2f%%, Command: %s\n",
                   p->pid, p->user, p->cpu_usage, p->mem_usage, p->command);
        }
    } else {
        printf("Échec de la mise à jour des processus distants.\n");
    }
    
    //LECTURE FICHIER CONFIG
    */
    

   
    
    return 0;
}