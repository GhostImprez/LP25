#ifndef UI_H
#define UI_H

#include <ncurses.h>
#include "manager.h" 

// Structure contenant l'état de l'affichage (position, scroll, onglet...)
typedef struct { 
    int h;          // Hauteur écran
    int w;          // Largeur écran
    int tab;        // Index de l'onglet (machine) actif
    int sel;        // Index de la ligne sélectionnée (curseur)
    int scroll;     // Offset de défilement (scrolling)
} ui_ctx_t;

// Initialise ncurses et les couleurs
void ui_init(ui_ctx_t *c);

// Ferme ncurses
void ui_end(void);

// Dessine l'interface complète
void ui_draw(ui_ctx_t *c, machine_t *m, int n);

// Gère les entrées clavier (flèches, F1)
int ui_input(ui_ctx_t *c, int n, int count);

#endif