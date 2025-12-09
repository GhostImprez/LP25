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

//option -h ou --help
void opt_help(){
    printf("INSERER UN TEXTE D'AIDE\n");
}

//option --dry-run
void opt_dry_run(machine_t *m){
    printf("mode dry run activé\n");
    update_local_processes(m); //teste l'accès aux processus locaux
}

//option -c ou --remote-config
void opt_remote_config(){
    printf("SOON\n");
    //à implémenter
}

//option -t ou --connexion-type
void opt_connexion_type(){
    printf("SOON\n");
    //à implémenter
}

//option -P ou --port
void opt_port(){
    printf("SOON\n");
    //à implémenter
}

//option -l ou --login
void opt_login(){
    printf("SOON\n");
    //à implémenter
}


//option -s ou --remote-server
void opt_remote_server(){
    printf("SOON\n");
    //à implémenter
}

//option -u ou --username
void opt_username(){
    printf("SOON\n");
    //à implémenter
}

//option -p ou --password
void opt_password(){
    printf("SOON\n");
    //à implémenter
}

//option -a ou --all
void opt_all(){
    printf("SOON\n");
    //à implémenter
}   



int main(){
    return 0;
}