projet complet dans la branche fusion


👤 Personne 1:  — Module process (gestion locale des processus)
Responsabilités principales

Implémenter toutes les fonctions liées aux processus locaux :

récupération de la liste des processus (lecture de /proc/)

extraction des infos (PID, utilisateur, CPU, mémoire, start time…)

fonctions pour :

pause (SIGSTOP)

arrêt (SIGTERM)

kill (SIGKILL)

reprise (SIGCONT)

redémarrage

Gestion des erreurs (PID inexistants, permissions, etc.)

Documentation Doxygen du module.

Tests avec Valgrind (fuites mémoire).

Livrables

process.c, process.h

Une API propre et stable pour le module manager.

👤 Personne 2 — Module network (SSH / Telnet / communication distante)
Responsabilités principales

Implémentation du système de connexion distante :

SSH (libssh, libssh2, ou exécution contrôlée de commandes ssh)

Telnet simple (connexion socket + commandes texte)

Gestion de la réception d’une liste de processus distants :

exécuter ps, ou envoyer un programme client sur la machine distante

Support de :

--remote-server

--remote-config

--login user@host

--dry-run

Sécurisation : vérification des permissions du fichier .config (chmod 600).

Gestion du port choisi (--port).

Livrables

network.c, network.h

Fonction stable de récupération des données distantes

Gestion détaillée des erreurs réseau (timeout, refus, mauvais PWD…)

👤 Personne 3 — Module ui (interface htop-like)
Responsabilités principales

Création de l’interface interactive avec ncurses :

affichage tableau htop-like

barre d’état

onglets pour chaque machine

rafraîchissement dynamique

Implémentation des touches :

F1 (aide)

F2 / F3 (onglet suivant / précédent)

F4 (recherche)

F5 (pause)

F6 (stop)

F7 (kill)

F8 (restart)

Gestion du redimensionnement terminal.

Gestion du mode sans interface pour --dry-run.

Livrables

ui.c, ui.h

Manuel d’utilisation pour F1

Interface propre, fluide, ergonomique

👤 Personne 4 — Module manager + Options CLI + Intégration finale
Responsabilités principales

Lecture et parsing des options :

--help

--dry-run

--remote-config

--login

--connexion-type

--port

--remote-server

--username, --password

--all

Orchestration entre :

process local

process distants

network

ui

Gestion du fichier .config

Centralisation des données de toutes les machines

Contrôleur pour les actions utilisateur (pauser, kill, restart)

Gestion des structures de données globales (listes, tableaux, maps…)

Livrables

manager.c, manager.h

main.c

Makefile

Documentation du parcours d’exécution
