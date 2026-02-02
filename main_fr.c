#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define LARGEUR 11
#define HAUTEUR 11
#define MAX_SALLES 15

#define NORD 0
#define EST 1
#define SUD 2
#define OUEST 3

typedef struct {
    int id;
    int portes[4]; // ID des salles dans chaque direction (-1 si pas de porte)
    int a_ennemi;  // 1 true, 0 false
    int a_tresor;  // 1 true, 0 false
    int a_azatoth; // 1 true, 0 false
    int ennemi_x;
    int ennemi_y;
    int tresor_x;
    int tresor_y;
    int portes_verrouillees[4]; // 1 Fermer, 0 Ouvert,
} Salle;

typedef struct {
    int x;
    int y;
    int pv;
    int salle_actuelle;
    int force; // boost de force fait plus de dégâts
    int defense; // boost de defense subis moins de dégâts
    int cles; // Nombre de clés
} Chevalier;

Salle plan[MAX_SALLES]; // Tableau des salles
Chevalier joueur;

void generer_contenu_salle(int id);
void combat();
void deplacement_enemies();
void afficher_salle();
void afficher_statut();
void initialiser_joueur();
void initialiser_plan();
void charger_plan(const char* nom_fichier);
void generer_donjon_aleatoire();
int char_to_dir(char c);
void sauvegarder_partie(const char* nom_fichier);
int charger_sauvegarde(const char* nom_fichier);

// Initialise le plan des salles
void initialiser_plan() {
    for (int i = 0; i < MAX_SALLES; i++) {
        plan[i].id = i;
        for (int d = 0; d < 4; d++) {
            plan[i].portes[d] = -1; // Par défaut pas de porte
            plan[i].portes_verrouillees[d] = 0; // Par défaut portes non verrouillées
        }
        // Contenu de la salle par défaut vide
        plan[i].a_ennemi = 0;
        plan[i].a_tresor = 0;
        plan[i].a_azatoth = 0;
        plan[i].ennemi_x = 1;
        plan[i].ennemi_y = 1;
        plan[i].tresor_x = 2;
        plan[i].tresor_y = 2;
    }
}

// Initialise le joueur
// Position par defaut, PV, etc.
void initialiser_joueur() {
    joueur.pv = 100;
    joueur.salle_actuelle = 1; // Départ salle 1
    joueur.x = 5;
    joueur.y = 5;
    joueur.cles = 0;
    joueur.force = 0;
    joueur.defense = 0;
}

// Convertit un char 'N','S','E','O' en int
int char_to_dir(char c) {
    if (c == 'N') return NORD;
    if (c == 'E') return EST;
    if (c == 'S') return SUD;
    if (c == 'O') return OUEST;
    return -1;
}

// Lit le fichier plan.pln
void charger_plan(const char* nom_fichier) {
    FILE* file = fopen(nom_fichier, "r");
    if (!file) {
        printf("Erreur: ouverture %s\n", nom_fichier);
        exit(1);
    }
    
    int s1, s2; // ID des salles
    char d1_char, d2_char; // Directions
    

    // format attendu : 1.S-2.N;
    // TODO regénére le contenu des memes salles plusieurs fois
    while (fscanf(file, "%d.%c-%d.%c;", &s1, &d1_char, &s2, &d2_char) == 4) {
        int dir1 = char_to_dir(d1_char); // Direction de s1 vers s2
        int dir2 = char_to_dir(d2_char); // Direction de s2 vers s1
        
        if (dir1 != -1 && dir2 != -1 && s1 < MAX_SALLES && s2 < MAX_SALLES) {
            plan[s1].portes[dir1] = s2; // s1 vers s2
            plan[s2].portes[dir2] = s1; // s2 vers s1
            
            generer_contenu_salle(s1); // Génère contenu pour s1
            generer_contenu_salle(s2); // Génère contenu pour s2
        }
    }
    
    // On place Azatoth dans la salle 7 
    plan[7].a_azatoth = 1;
    generer_contenu_salle(7); 
    
    // Verrouillage des portes
    // 2 et 3
    plan[2].portes_verrouillees[EST] = 1; // 2 vers 3 (E)
    plan[3].portes_verrouillees[OUEST] = 1; // 3 vers 2 (O)
    
    // 6 et 7
    plan[6].portes_verrouillees[EST] = 1; // 6 vers 7 (E)
    plan[7].portes_verrouillees[OUEST] = 1; // 7 vers 6 (O)
    
    // 4 et 5
    plan[4].portes_verrouillees[SUD] = 1; // 4 vers 5 (S)
    plan[5].portes_verrouillees[NORD] = 1; // 5 vers 4 (N)

    fclose(file);
}

void generer_contenu_salle(int id) {
    // Position aléatoire pour Azatoth si dans la salle
    if (plan[id].a_azatoth) {
        plan[id].ennemi_x = 5;
        plan[id].ennemi_y = 5;
        return; 
    }

    // 1 chance sur 2 d'avoir un ennemi
    if (rand() % 2 == 0) {
        plan[id].a_ennemi = 1;
        plan[id].ennemi_x = (rand() % 9) + 1; 
        plan[id].ennemi_y = (rand() % 9) + 1;
    }
    
    // 1 chance sur 3 d'avoir un trésor
    if (rand() % 3 == 0) {
        plan[id].a_tresor = 1;
        plan[id].tresor_x = (rand() % 9) + 1;
        plan[id].tresor_y = (rand() % 9) + 1;
        // Eviter superposition avec ennemi
        while (plan[id].a_ennemi && plan[id].tresor_x == plan[id].ennemi_x && plan[id].tresor_y == plan[id].ennemi_y) {
            plan[id].tresor_x = (rand() % 9) + 1;
            plan[id].tresor_y = (rand() % 9) + 1;
        }
    }
}

void deplacement_enemies() {
    Salle* s = &plan[joueur.salle_actuelle];
    if (!s->a_ennemi && !s->a_azatoth) return;

    // Déplacement aléatoire 0=Haut, 1=Bas, 2=Gauche, 3=Droite, 4=Rien
    int move = rand() % 5;
    int dx = 0, dy = 0;
    
    switch (move) {
        case 0: dy = -1; break;
        case 1: dy = 1; break;
        case 2: dx = -1; break;
        case 3: dx = 1; break;
        case 4: return;
    }

    // Nouvelles coordonnées
    int new_x = s->ennemi_x + dx;
    int new_y = s->ennemi_y + dy;

    // Vérifier collision avec les murs extérieurs 
    int est_bord = (new_x == 0 || new_x == LARGEUR-1 || new_y == 0 || new_y == HAUTEUR-1);
    
    // L'ennemi ne peut pas passer par les portes, donc on bloque sur tous les bords
    if (est_bord) {
        return;
    }
    
    // Vérifier que l'ennemi ne marche pas sur le trésor
    if (s->a_tresor && new_x == s->tresor_x && new_y == s->tresor_y) {
        return;
    }

    // Déplacement validé
    s->ennemi_x = new_x;
    s->ennemi_y = new_y;
    
    // Si l'ennemi marche sur le joueur -> Combat
    if (s->ennemi_x == joueur.x && s->ennemi_y == joueur.y) {
        combat();
    }
}


// Affichage général
void afficher_statut() {
    printf("\n----- STATUT -----\n");
    printf("PV: %d | Salle: %d | Force: %d | Défense: %d | Clés: %d\n", 
        joueur.pv,
        joueur.salle_actuelle, 
        joueur.force, 
        joueur.defense, 
        joueur.cles);
    printf("Commandes : Z (Haut), Q (Gauche), S (Bas), D (Droite), P (Sauvegarder), X (Quitter)\n");
}

// Affiche la salle actuelle
void afficher_salle() {
    Salle* s = &plan[joueur.salle_actuelle];
    
    printf("\nPiece %d :\n", s->id);
    printf("-------------------------\n");

    for (int y = 0; y < HAUTEUR; y++) {
        printf("| ");
        for (int x = 0; x < LARGEUR; x++) {
            int personnage_joueur = (x == joueur.x && y == joueur.y); // Position du joueur
            // Vérifie si un ennemi ou coffre est présent aux coordonnées (x,y)
            int personnage_ennemi = (s->a_ennemi && x == s->ennemi_x && y == s->ennemi_y && !s->a_azatoth);
            int personnage_boss = (s->a_azatoth && x == s->ennemi_x && y == s->ennemi_y);
            int coffre_tresor = (s->a_tresor && x == s->tresor_x && y == s->tresor_y);
            
            // Portes (Milieu des murs)
            int porte_n = (y == 0 && x == LARGEUR/2);
            int porte_s = (y == HAUTEUR-1 && x == LARGEUR/2);
            int porte_e = (x == LARGEUR-1 && y == HAUTEUR/2);
            int porte_o = (x == 0 && y == HAUTEUR/2);
            
            if (personnage_joueur) {
                printf("C "); // Chevalier
            } else if (personnage_boss) {
                printf("AZ"); // Z pour Azatoth
            } else if (personnage_ennemi) {
                printf("E "); // Ennemi
            } else if (coffre_tresor) {
                printf("T "); // Trésor
            } else if (porte_n) {
                char c = '.';
                if (s->portes[NORD] != -1) 
                    c = s->portes_verrouillees[NORD] ? 'F' : 'O'; // -1 pas de porte , 0 ouvert, 1 fermer
                printf("%c ", c);
            } else if (porte_s) {
                char c = '.';
                if (s->portes[SUD] != -1) 
                    c = s->portes_verrouillees[SUD] ? 'F' : 'O';
                printf("%c ", c);
            } else if (porte_e) {
                char c = '.';
                if (s->portes[EST] != -1) 
                    c = s->portes_verrouillees[EST] ? 'F' : 'O';
                printf("%c ", c);
            } else if (porte_o) {
                char c = '.';
                if (s->portes[OUEST] != -1) 
                    c = s->portes_verrouillees[OUEST] ? 'F' : 'O';
                printf("%c ", c);
            } else {
                printf(". ");
            }
        }
        printf("|\n");
    }
    printf("-------------------------\n");
}

void combat() {
    Salle* s = &plan[joueur.salle_actuelle];
    
    printf("\n!!! COMBAT ENCLENCHE !!!\n");
    // Boss 80PV, sinon 30
    int pv_ennemi;
    if (s->a_azatoth) {
        pv_ennemi = 80;
    } else {
        pv_ennemi = 30;
    }

    while (pv_ennemi > 0 && joueur.pv > 0) {
        printf("\nVotre choix (1: Pierre, 2: Feuille, 3: Ciseaux) : ");
        int choix_joueur;
        if (scanf("%d", &choix_joueur) != 1) { // Si l'entrée n'est pas un entier
            int c; 
            while((c = getchar()) != '\n'); // vide le buffer
            continue;
        }
        
        if (choix_joueur < 1 || choix_joueur > 3) {
            printf("Erreur : choix invalide. Veuillez entrer 1, 2 ou 3.\n");
            continue;
        }
        
        int choix_ennemi = (rand() % 3) + 1;
        
        char *coups[] = {"", "Pierre", "Feuille", "Ciseaux"};
        printf("Vous: %s vs Ennemi: %s -> ", coups[choix_joueur], coups[choix_ennemi]);

        // Calcul des dégâts
        int degats_ennemi_base = s->a_azatoth ? 20 : 10;
        int degats_joueur = 10 + joueur.force; // Dégâts de base 10 + force
        int degats_ennemi = degats_ennemi_base - joueur.defense; // Dégâts subis réduits par défense

        if (degats_ennemi < 0) degats_ennemi = 0;

        if (choix_joueur == choix_ennemi) {
            printf("Egalite !\n");

        } else if ((choix_joueur == 1 && choix_ennemi == 3) || (choix_joueur == 2 && choix_ennemi == 1) || (choix_joueur == 3 && choix_ennemi == 2)) {
            printf("Gagne ! Ennemi perd %d PV.\n", degats_joueur);
            pv_ennemi -= degats_joueur;

        } else {
            printf("Perdu ! Vous perdez %d PV.\n", degats_ennemi);
            joueur.pv -= degats_ennemi;
        }
        printf("PV Ennemi: %d | PV Joueur: %d\n", pv_ennemi, joueur.pv);
    }
    if (joueur.pv <= 0) {
        printf("Vous etes mort...\n");
        exit(0);
    } else {
        printf("Victoire !\n");
        if (s->a_azatoth) {
            printf("\nBRAVO ! Vous avez vaincu Azatoth et sauvé la région !\n");
            exit(0);
        } else {
            // Butin ennemi classique
            int butin = rand() % 3; // 0: Rien, 1: Clé, 2: PV
            if (butin == 1) {
                printf("L'ennemi laisse tomber une cle !\n");
                joueur.cles++;
            } else if (butin == 2) {
                printf("Vous recuperez un peu de vie ! +10 PV.\n");
                joueur.pv += 10;
                if (joueur.pv > 100) joueur.pv = 100;
            } else {
                printf("L'ennemi n'avait rien sur lui.\n");
            }
            printf("\nAppuyez sur Entree pour continuer...");
            getchar(); getchar();
        }
        s->a_ennemi = 0; // Ennemi vaincu
    }
}

// Gère le déplacement du joueur
void deplacer(char direction) {
    int dx = 0, dy = 0;
    
    switch(direction) {
        case 'Z': case 'z': dy = -1; break;
        case 'S': case 's': dy = 1; break;
        case 'Q': case 'q': dx = -1; break;
        case 'D': case 'd': dx = 1; break;
        default: return;
    }
    
    int new_x = joueur.x + dx;
    int new_y = joueur.y + dy;
    
    Salle* s = &plan[joueur.salle_actuelle];

    // Vérifier collision avec les murs extérieurs
    int est_bord = (new_x == 0 || new_x == LARGEUR-1 || new_y == 0 || new_y == HAUTEUR-1);
    
    // Indique si la position correspond à une porte
    int est_porte_n = (new_x == 5 && new_y == 0);
    int est_porte_s = (new_x == 5 && new_y == HAUTEUR-1);
    int est_porte_o = (new_x == 0 && new_y == 5);
    int est_porte_e = (new_x == LARGEUR-1 && new_y == 5);
    
    int est_une_porte = (est_porte_n || est_porte_s || est_porte_o || est_porte_e);

    // Si c'est un mur mais pas une porte, on bloque
    if (est_bord && !est_une_porte) {
        return;
    }
    // Vérifier collision avec l'ennemi
    if (new_x == s->ennemi_x && new_y == s->ennemi_y && (s->a_ennemi || s->a_azatoth)) {
        combat();
    }

    // Gestion des portes
    if (est_une_porte) {
        int direction_porte = -1;

        if (est_porte_n) direction_porte = NORD;
        else if (est_porte_s) direction_porte = SUD;
        else if (est_porte_e) direction_porte = EST;
        else if (est_porte_o) direction_porte = OUEST;
        
        // Vérifie si une porte existe différent de -1
        if (s->portes[direction_porte] != -1) {
            // Vérifie si la porte est fermée
            if (s->portes_verrouillees[direction_porte]) {
            if (joueur.cles > 0) { // Si le joueur possède une clé
                printf("Porte verrouillée 'F'. Vous utilisez une clé !\n");
                joueur.cles--;
                s->portes_verrouillees[direction_porte] = 0; // Déverrouille la porte
                int salle_dest = s->portes[direction_porte]; // Identifie la salle de destination
                int dir_opposee = (direction_porte + 2) % 4; // prend la porte opposée
                plan[salle_dest].portes_verrouillees[dir_opposee] = 0; // Déverrouille la porte dans l'autre salle
                printf("\nAppuyez sur Entree pour continuer...");
                getchar(); getchar();
            } else {
                printf("Porte verrouillée 'F'. Il vous faut une clé.\n");
                printf("\nAppuyez sur Entree pour continuer...");
                getchar(); getchar();
                return;
            }
            }
            
            // changement de salle
            joueur.salle_actuelle = s->portes[direction_porte];
            if (direction_porte == NORD) { joueur.x = LARGEUR/2; joueur.y = HAUTEUR - 2; }
            else if (direction_porte == SUD) { joueur.x = LARGEUR/2; joueur.y = 1; }
            else if (direction_porte == EST) { joueur.x = 1; joueur.y = HAUTEUR/2; }
            else if (direction_porte == OUEST) { joueur.x = LARGEUR - 2; joueur.y = HAUTEUR/2; }
            printf("Vous entrez dans la salle %d.\n", joueur.salle_actuelle);
            getchar(); getchar();
        }
        return;
    }
    
    // Déplacement normal
    joueur.x = new_x;
    joueur.y = new_y;

    // Ramasser trésor
    if (s->a_tresor && joueur.x == s->tresor_x && joueur.y == s->tresor_y) {
        int type_tresor = rand() % 3; // 0: PV, 1: Force, 2: Défense
        if (type_tresor == 0) {
            printf("Vous trouvez une potion ! +20 PV.\n");
            joueur.pv += 20;
            if (joueur.pv > 100) joueur.pv = 100;
        } else if (type_tresor == 1) {
            printf("Vous trouvez une longue épée ! Force +2.\n");
            joueur.force += 2;
        } else {
            printf("Vous trouvez un bouclier ! Défense +1.\n");
            joueur.defense += 1;
        }
        s->a_tresor = 0;
        printf("\nAppuyez sur Entree pour continuer...");
        getchar(); getchar();
    }
}

void sauvegarder_partie(const char* nom_fichier) {
    // Ecriture en binaire ("wb")
    FILE* file = fopen(nom_fichier, "wb");
    if (!file) {
        printf("Erreur: impossible de creer la sauvegarde.\n");
        return;
    }
    
    // Écrit la structure joueur dans le fichier de taille Chevalier
    fwrite(&joueur, sizeof(Chevalier), 1, file);
    // Écrit le tableau plan complet dans le fichier de taille Salle * MAX_SALLES
    fwrite(plan, sizeof(Salle), MAX_SALLES, file);
    
    fclose(file);
    printf("Partie sauvegardee !\n");
}

int charger_sauvegarde(const char* nom_fichier) {
    // Lecture en binaire ("rb")
    FILE* file = fopen(nom_fichier, "rb");
    if (!file) {
        return 0;
    }
    
    // Lit la structure Chevalier depuis le fichier et vérifie si il y a une erreur
    if (fread(&joueur, sizeof(Chevalier), 1, file) != 1) {
        fclose(file);
        return 0;
    }
    // Lit la structure Salle MAX_SALLES dois depuis le fichier et vérifie si il y a une erreur
    if (fread(plan, sizeof(Salle), MAX_SALLES, file) != MAX_SALLES) {
        fclose(file);
        return 0;
    }
    
    fclose(file);
    return 1;
}

void generer_donjon_aleatoire() {
    printf("Génération du donjon aléatoire...\n");
    initialiser_plan();
    
    int visited[MAX_SALLES];
    for(int i=0; i<MAX_SALLES; i++) visited[i] = 0;
    
    // On commence salle 1
    int current_rooms = 0;
    int start_room = 1;
    visited[start_room] = 1;
    current_rooms++;
    
    // On veut connecter entre 8 et MAX_SALLES-1 salles
    int target_rooms = 8 + (rand() % (MAX_SALLES - 9)); 
    
    while (current_rooms < target_rooms) {
        // Choisir une salle déjà visitée au hasard
        int r1 = -1;
        do {
            r1 = rand() % MAX_SALLES;
        } while (!visited[r1]);
        
        // Choisir une direction au hasard
        int dir = rand() % 4; // 0-3
        
        // Si la porte est libre
        if (plan[r1].portes[dir] == -1) {
            // Trouver une salle non visitée
            int r2 = -1;
            // On cherche une salle libre
            for (int k = 1; k < MAX_SALLES; k++) {
                if (!visited[k]) {
                    r2 = k;
                    break;
                }
            }
            
            if (r2 != -1) {
                // Connexion
                int opp = (dir + 2) % 4;
                plan[r1].portes[dir] = r2;
                plan[r2].portes[opp] = r1;
                
                visited[r2] = 1;
                current_rooms++;
                
                // Générer contenu
                generer_contenu_salle(r1);
                generer_contenu_salle(r2);
                
                // Chance de verrouiller (10%)
                if (rand() % 10 == 0) {
                    plan[r1].portes_verrouillees[dir] = 1;
                    plan[r2].portes_verrouillees[opp] = 1;
                }
            }
        }
    }
    
    // Placer Azatoth dans la dernière salle ajoutée ou une salle lointaine
    // Pour simplifier, on prend une salle visitée au hasard qui n'est pas la 1
    int boss_room = -1;
    while (boss_room == -1 || boss_room == 1) {
        int r = rand() % MAX_SALLES;
        if (visited[r]) boss_room = r;
    }
    
    // Nettoyer la salle du boss
    plan[boss_room].a_ennemi = 0;
    plan[boss_room].a_tresor = 0;
    plan[boss_room].a_azatoth = 1;
    generer_contenu_salle(boss_room); // Replacer le boss correctement
    
    printf("Donjon généré ! %d salles connectées. Boss en salle %d.\n", current_rooms, boss_room);
}

int main() {
    srand(time(NULL));
    
    // Initialisation
    initialiser_plan();
    initialiser_joueur();
    
    printf("Bienvenue Chevalier. Trouvez Azatoth !\n");
    printf("Charger une sauvegarde ? (O/N) : ");
    char choix;
    scanf(" %c", &choix);
    int partie_chargee = 0;
    
    if (choix == 'O' || choix == 'o') {
        if (charger_sauvegarde("sauvegarde.sav")) {
            printf("Sauvegarde chargee !\n");
            partie_chargee = 1;
        } else {
            printf("Aucune sauvegarde trouvee.\n");
        }
    }
    
    if (!partie_chargee) {
        printf("Voulez-vous un donjon aléatoire ? (O/N) : ");
        char choix_donjon;
        scanf(" %c", &choix_donjon);
        if (choix_donjon == 'O' || choix_donjon == 'o') {
            generer_donjon_aleatoire();
        } else {
            charger_plan("plan_base.pln");
        }
    }

    printf("Appuyez sur Entree pour commencer...");
    getchar(); getchar();

    // Boucle du jeu
    char commande;
    while(1) {
        system("clear");
        afficher_statut();
        afficher_salle();
        
        printf("-> ");
        scanf(" %c", &commande);
        
        if (commande == 'X' || commande == 'x') {
            break;
        } else if (commande == 'P' || commande == 'p') {
            sauvegarder_partie("sauvegarde.sav");
            printf("\nAppuyez sur Entree pour continuer...");
            getchar(); getchar();
        } else {
            deplacer(commande);
            deplacement_enemies();
        }
    }
    
    return 0;
}
