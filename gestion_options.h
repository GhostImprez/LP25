#ifndef GESTION_OPTIONS_H
#define GESTION_OPTIONS_H
#include "process.h"

//prototypes des fonctions de gestion des options
void opt_help();
void opt_dry_run(machine_t *m);
void opt_remote_config();
void opt_connexion_type();
void opt_port();
void opt_login();
void opt_remote_server();
void opt_username();
void opt_password();
void opt_all();


#endif