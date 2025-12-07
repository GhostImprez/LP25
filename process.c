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

*/



int main() {
    
}