#include "ui.h"
#include <stdlib.h>
#include <string.h>


void ui_init(ui_ctx_t *c) { 
    // Démarrage standard de ncurses
    initscr(); 
    cbreak(); 
    noecho(); 
    keypad(stdscr, TRUE); 
    curs_set(0);         
    start_color();

    // Définition des paires de couleurs
    init_pair(1, COLOR_WHITE, COLOR_BLACK); // Texte standard
    init_pair(2, COLOR_BLACK, COLOR_CYAN);  // En-têtes 
    init_pair(3, COLOR_BLACK, COLOR_GREEN); // Sélection 

    // Récupération taille écran et initialisation variables
    getmaxyx(stdscr, c->h, c->w); 
    c->tab = 0; 
    c->sel = 0; 
    c->scroll = 0;
}

void ui_end(void) { 
    endwin(); // Restaure le terminal normal
}

void ui_draw(ui_ctx_t *c, machine_t *m, int n) {
    erase(); 

    // Si aucune machine, on affiche un message et on quitte
    if (n <= 0 || !m) {
        mvprintw(0, 0, "Aucune machine disponible.");
        refresh();
        return;
    }

    // Gestion de l'onglet circulaire (si on dépasse le nb de machines, on revient à 0)
    if (c->tab >= n) c->tab = 0;

    // Pointeur vers la machine actuelle pour simplifier le code
    machine_t *current_machine = &m[c->tab];
    int count = current_machine->processes.count; 

    attron(COLOR_PAIR(2)); 
    mvprintw(0, 0, " Machine: %s ", current_machine->name); 
    // Remplit le reste de la ligne avec des espaces vides cyan
    for(int i = strlen(current_machine->name) + 10; i < c->w; i++) addch(' '); 
    attroff(COLOR_PAIR(2));

    mvprintw(2, 1, "PID   USER       STATE CPU%%  MEM%%  COMMAND");

    // Liste des processus 
    // On affiche autant de lignes que l'écran le permet
    for(int i = 0; i < c->h - 4; i++) {
        int idx = c->scroll + i; 

        // Si on dépasse le nombre de processus, on arrête d'afficher
        if(idx >= count) break;
        
        // Pointeur vers le processus spécifique
        process_t *p = &current_machine->processes.list[idx];

        // Changement de couleur, si sélectionné
        if(idx == c->sel) attron(COLOR_PAIR(3));
        
        mvprintw(3 + i, 1, "%-5d %-10s %c     %-5.1f %-5.1f %s", 
                 p->pid, 
                 p->user, 
                 p->state,
                 p->cpu_usage, 
                 p->mem_usage, 
                 p->command);
                 
        attroff(COLOR_PAIR(3));
    }

    // Pied de page 
    mvprintw(c->h - 1, 1, "F2: Onglet Suivant |F5: Pause |F6: Arrêt |F7: Kill |F8: Reprise | q: Quitter | Fleches: Naviguer");
    refresh();
}

int ui_input(ui_ctx_t *c, int n, int count) {
    int k = getch(); // Attend une touche

    // Gestion Onglets (F2)
    if(k == KEY_F(2)) { 
        c->tab = (c->tab + 1) % n; 
        c->sel = 0;    // Remet la sélection en haut
        c->scroll = 0; // Remet le scroll en haut
    }

    // Gestion Flèche BAS
    if(k == KEY_DOWN && c->sel < count - 1) {
        c->sel++;
        // Si le curseur descend plus bas que l'écran, on scrolle
        if(c->sel >= c->scroll + (c->h - 4)) {
            c->scroll++;
        }
    }

    // Gestion Flèche HAUT
    if(k == KEY_UP && c->sel > 0) {
        c->sel--;
        // Si le curseur monte plus haut que l'écran, on scrolle inversement
        if(c->sel < c->scroll) {
            c->scroll--;
        }
    }

    return k; // Renvoie la touche 
}