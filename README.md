Thibaut Castillo L2A

# Projet Labyrinthe

## Description du Projet

Jeu d'aventure en C dans lequel un chevalier doit explorer un labyrinthe composé de 13 salles interconnectées pour trouver et vaincre le boss final : Azatoth.

## Fonctionnalités et Mécaniques de Jeu
- **Exploration** : Déplacement dans 13 salles différentes via des portes (N/S/E/O).
- **Combat** : Système de combat au pierre-feuille-ciseaux contre les ennemis.
- **Gestion de ressources** : PV, force, défense et clés.
- **Portes verrouillées** : Certaines portes nécessitent des clés pour être déverrouillées.
- **Boss final** : Azatoth dans la salle 7 (80 PV au lieu de 30 pour les ennemis normaux).
- **Ennemis** : Apparition aléatoire dans les salles.
- **Trésors** : Coffres contenant potions (PV), épées (force) ou boucliers (défense).
- **Clés** : Obtenues en tuant des ennemis, pour ouvrir les portes.
- **Sauvegarde/Chargement** : Système de sauvegarde dans un fichier binaire pour reprendre la partie.

## Contrôles
- **Z** : Haut
- **Q** : Gauche  
- **S** : Bas
- **D** : Droite
- **P** : Sauvegarder
- **X** : Quitter

## Structure du Labyrinthe

- Grille 11x11 par salle
- 15 salles maximum (13 utilisées)
- Génération aléatoire du contenu des salles
- Fichier `plan.pln` pour définir le plan du labyrinthe
- Fichier `sauvegarde.sav` pour les sauvegardes

## Compilation et Exécution

```bash
gcc -Wall -Wextra -Wunused -o main main.c
./main
```

## Plan du Labyrinthe

1.S-2.N;
2.O-10.E;
2.E-3.O;
2.S-8.N;
10.S-9.N;
9.E-8.O;
3.E-11.O;
3.S-4.N;
11.S-12.N;
4.S-5.N;
5.E-13.O;
12.S-13.N;
5.S-6.N;
6.E-7.O;

```
       [1]
        |
  [10]--[2]--[3]--[11]
    |    |    |     |
   [9]--[8]  [4]  [12]
              |    |
             [5]--[13]
              |
             [6]--[7] <-- Boss Azatoth
```

### Portes verrouillées
- Salle 2 <-> Salle 3
- Salle 4 <-> Salle 5
- Salle 6 <-> Salle 7

---

# Labyrinth of Azatoth Project

## Project Description

An adventure game in C where a knight must explore a labyrinth of 13 interconnected rooms to find and defeat the final boss: Azatoth.

## Game Features and Mechanics
- **Exploration**: Moving through 13 different rooms via doors (N/S/E/W).
- **Combat**: Rock-paper-scissors combat system against enemies.
- **Resource Management**: HP, strength, defense, and keys.
- **Locked Doors**: Some doors require keys to be unlocked.
- **Final Boss**: Azatoth in room 7 (80 HP instead of 30 for normal enemies).
- **Enemies**: Random appearance in rooms.
- **Treasures**: Chests containing potions (HP), swords (strength), or shields (defense).
- **Keys**: Obtained by killing enemies, used to open doors.
- **Save/Load**: Binary file save system to resume the game.

## Controls
- **Z**: Up
- **Q**: Left  
- **S**: Down
- **D**: Right
- **P**: Save
- **X**: Quit

## Labyrinth Structure

- 11x11 grid per room
- Maximum 15 rooms (13 used)
- Random generation of room content
- `plan.pln` file to define the labyrinth layout
- `sauvegarde.sav` (or `save.sav`) file for saves

## Compilation and Execution

```bash
gcc -Wall -Wextra -Wunused -o main main.c
./main
```

## Labyrinth Layout

1.S-2.N;
2.O-10.E;
2.E-3.O;
2.S-8.N;
10.S-9.N;
9.E-8.O;
3.E-11.O;
3.S-4.N;
11.S-12.N;
4.S-5.N;
5.E-13.O;
12.S-13.N;
5.S-6.N;
6.E-7.O;

```
       [1]
        |
  [10]--[2]--[3]--[11]
    |    |    |     |
   [9]--[8]  [4]  [12]
              |    |
             [5]--[13]
              |
             [6]--[7] <-- Boss Azatoth
```

### Locked Doors
- Room 2 <-> Room 3
- Room 4 <-> Room 5
- Room 6 <-> Room 7
