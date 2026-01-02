#ifndef NETWORK_H
#define NETWORK_H
#include "manager.h"
int parse_login(const char *login_str, int port, machine_t *m);
int parse_username_host(const char *username_str, const char *host_str, int port, machine_t *m);
void lecture_fichier_config(const char *chemin, manager_t *mgr);
int update_remote_processes(machine_t *m);


#endif