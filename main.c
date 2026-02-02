#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define WIDTH 11
#define HEIGHT 11
#define MAX_ROOMS 15

#define NORTH 0
#define EAST 1
#define SOUTH 2
#define WEST 3

typedef struct {
    int id;
    int doors[4]; // Room IDs in each direction (-1 if no door)
    int has_enemy;  // 1 true, 0 false
    int has_treasure;  // 1 true, 0 false
    int has_azatoth; // 1 true, 0 false
    int enemy_x;
    int enemy_y;
    int treasure_x;
    int treasure_y;
    int locked_doors[4]; // 1 Closed, 0 Open
} Room;

typedef struct {
    int x;
    int y;
    int hp;
    int current_room;
    int strength; // strength boost deals more damage
    int defense; // defense boost reduces damage taken
    int keys; // Number of keys
} Knight;

Room map[MAX_ROOMS]; // Array of rooms
Knight player;

void generate_room_content(int id);
void combat();
void move_enemies();
void display_room();
void display_status();
void initialize_player();
void initialize_map();
void load_map(const char* filename);
int char_to_dir(char c);void generate_random_dungeon();void save_game(const char* filename);
int load_save(const char* filename);

// Initializes the room map
void initialize_map() {
    for (int i = 0; i < MAX_ROOMS; i++) {
        map[i].id = i;
        for (int d = 0; d < 4; d++) {
            map[i].doors[d] = -1; // No door by default
            map[i].locked_doors[d] = 0; // Doors unlocked by default
        }
        // Room content empty by default
        map[i].has_enemy = 0;
        map[i].has_treasure = 0;
        map[i].has_azatoth = 0;
        map[i].enemy_x = 1;
        map[i].enemy_y = 1;
        map[i].treasure_x = 2;
        map[i].treasure_y = 2;
    }
}

// Initializes the player
// Default position, HP, etc.
void initialize_player() {
    player.hp = 100;
    player.current_room = 1; // Start in room 1
    player.x = 5;
    player.y = 5;
    player.keys = 0;
    player.strength = 0;
    player.defense = 0;
}

// Converts a char 'N','S','E','W' (formerly 'O') to int
int char_to_dir(char c) {
    if (c == 'N') return NORTH;
    if (c == 'E') return EAST;
    if (c == 'S') return SOUTH;
    if (c == 'O' || c == 'W') return WEST; // Keep Support for 'O' from plan.pln but support 'W'
    return -1;
}

// Reads the plan.pln file
void load_map(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error: opening %s\n", filename);
        exit(1);
    }
    
    int s1, s2; // Room IDs
    char d1_char, d2_char; // Directions
    

    // expected format : 1.S-2.N;
    while (fscanf(file, "%d.%c-%d.%c;", &s1, &d1_char, &s2, &d2_char) == 4) {
        int dir1 = char_to_dir(d1_char); // Direction from s1 to s2
        int dir2 = char_to_dir(d2_char); // Direction from s2 to s1
        
        if (dir1 != -1 && dir2 != -1 && s1 < MAX_ROOMS && s2 < MAX_ROOMS) {
            map[s1].doors[dir1] = s2; // s1 to s2
            map[s2].doors[dir2] = s1; // s2 to s1
            
            generate_room_content(s1); // Generates content for s1
            generate_room_content(s2); // Generates content for s2
        }
    }
    
    // Place Azatoth in room 7 
    map[7].has_azatoth = 1;
    generate_room_content(7); 
    
    // Door locking
    // 2 and 3
    map[2].locked_doors[EAST] = 1; // 2 to 3 (E)
    map[3].locked_doors[WEST] = 1; // 3 to 2 (W)
    
    // 6 and 7
    map[6].locked_doors[EAST] = 1; // 6 to 7 (E)
    map[7].locked_doors[WEST] = 1; // 7 to 6 (W)
    
    // 4 and 5
    map[4].locked_doors[SOUTH] = 1; // 4 to 5 (S)
    map[5].locked_doors[NORTH] = 1; // 5 to 4 (N)

    fclose(file);
}

void generate_room_content(int id) {
    // Random position for Azatoth if in the room
    if (map[id].has_azatoth) {
        map[id].enemy_x = 5;
        map[id].enemy_y = 5;
        return; 
    }

    // 1 in 2 chance to have an enemy
    if (rand() % 2 == 0) {
        map[id].has_enemy = 1;
        map[id].enemy_x = (rand() % 9) + 1; 
        map[id].enemy_y = (rand() % 9) + 1;
    }
    
    // 1 in 3 chance to have a treasure
    if (rand() % 3 == 0) {
        map[id].has_treasure = 1;
        map[id].treasure_x = (rand() % 9) + 1;
        map[id].treasure_y = (rand() % 9) + 1;
        // Avoid overlap with enemy
        while (map[id].has_enemy && map[id].treasure_x == map[id].enemy_x && map[id].treasure_y == map[id].enemy_y) {
            map[id].treasure_x = (rand() % 9) + 1;
            map[id].treasure_y = (rand() % 9) + 1;
        }
    }
}

void move_enemies() {
    Room* s = &map[player.current_room];
    if (!s->has_enemy && !s->has_azatoth) return;

    // Random movement 0=Up, 1=Down, 2=Left, 3=Right, 4=Stay
    int move = rand() % 5;
    int dx = 0, dy = 0;
    
    switch (move) {
        case 0: dy = -1; break;
        case 1: dy = 1; break;
        case 2: dx = -1; break;
        case 3: dx = 1; break;
        case 4: return;
    }

    // New coordinates
    int new_x = s->enemy_x + dx;
    int new_y = s->enemy_y + dy;

    // Check collision with outer walls 
    int is_border = (new_x == 0 || new_x == WIDTH-1 || new_y == 0 || new_y == HEIGHT-1);
    
    // Enemy cannot go through doors, so we block on all borders
    if (is_border) {
        return;
    }
    
    // Check if enemy walks on treasure
    if (s->has_treasure && new_x == s->treasure_x && new_y == s->treasure_y) {
        return;
    }

    // Movement validated
    s->enemy_x = new_x;
    s->enemy_y = new_y;
    
    // If enemy walks on player -> Combat
    if (s->enemy_x == player.x && s->enemy_y == player.y) {
        combat();
    }
}


// General status display
void display_status() {
    printf("\n----- STATUS -----\n");
    printf("HP: %d | Room: %d | Strength: %d | Defense: %d | Keys: %d\n", 
        player.hp,
        player.current_room, 
        player.strength, 
        player.defense, 
        player.keys);
    printf("Commands: Z (Up), Q (Left), S (Down), D (Right), P (Save), X (Quit)\n");
}

// Displays the current room
void display_room() {
    Room* s = &map[player.current_room];
    
    printf("\nRoom %d:\n", s->id);
    printf("-------------------------\n");

    for (int y = 0; y < HEIGHT; y++) {
        printf("| ");
        for (int x = 0; x < WIDTH; x++) {
            int player_char = (x == player.x && y == player.y); // Player position
            // Check if enemy or chest is present at coordinates (x,y)
            int enemy_char = (s->has_enemy && x == s->enemy_x && y == s->enemy_y && !s->has_azatoth);
            int boss_char = (s->has_azatoth && x == s->enemy_x && y == s->enemy_y);
            int treasure_chest = (s->has_treasure && x == s->treasure_x && y == s->treasure_y);
            
            // Doors (Middle of walls)
            int door_n = (y == 0 && x == WIDTH/2);
            int door_s = (y == HEIGHT-1 && x == WIDTH/2);
            int door_e = (x == WIDTH-1 && y == HEIGHT/2);
            int door_w = (x == 0 && y == HEIGHT/2);
            
            if (player_char) {
                printf("K "); // Knight
            } else if (boss_char) {
                printf("AZ"); // AZ for Azatoth
            } else if (enemy_char) {
                printf("E "); // Enemy
            } else if (treasure_chest) {
                printf("T "); // Treasure
            } else if (door_n) {
                char c = '.';
                if (s->doors[NORTH] != -1) 
                    c = s->locked_doors[NORTH] ? 'L' : 'O'; // -1 no door, 0 open, 1 locked
                printf("%c ", c);
            } else if (door_s) {
                char c = '.';
                if (s->doors[SOUTH] != -1) 
                    c = s->locked_doors[SOUTH] ? 'L' : 'O';
                printf("%c ", c);
            } else if (door_e) {
                char c = '.';
                if (s->doors[EAST] != -1) 
                    c = s->locked_doors[EAST] ? 'L' : 'O';
                printf("%c ", c);
            } else if (door_w) {
                char c = '.';
                if (s->doors[WEST] != -1) 
                    c = s->locked_doors[WEST] ? 'L' : 'O';
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
    Room* s = &map[player.current_room];
    
    printf("\n!!! COMBAT ENGAGED !!!\n");
    // Boss 80HP, otherwise 30
    int enemy_hp;
    if (s->has_azatoth) {
        enemy_hp = 80;
    } else {
        enemy_hp = 30;
    }

    while (enemy_hp > 0 && player.hp > 0) {
        printf("\nYour choice (1: Rock, 2: Paper, 3: Scissors): ");
        int player_choice;
        if (scanf("%d", &player_choice) != 1) { // If input is not an integer
            int c; 
            while((c = getchar()) != '\n'); // clear buffer
            continue;
        }
        
        if (player_choice < 1 || player_choice > 3) {
            printf("Error: invalid choice. Please enter 1, 2, or 3.\n");
            continue;
        }
        
        int enemy_choice = (rand() % 3) + 1;
        
        char *moves[] = {"", "Rock", "Paper", "Scissors"};
        printf("You: %s vs Enemy: %s -> ", moves[player_choice], moves[enemy_choice]);

        // Damage calculation
        int enemy_base_damage = s->has_azatoth ? 20 : 10;
        int player_damage = 10 + player.strength; // Base damage 10 + strength
        int enemy_damage = enemy_base_damage - player.defense; // Damage taken reduced by defense

        if (enemy_damage < 0) enemy_damage = 0;

        if (player_choice == enemy_choice) {
            printf("Tie!\n");

        } else if ((player_choice == 1 && enemy_choice == 3) || (player_choice == 2 && enemy_choice == 1) || (player_choice == 3 && enemy_choice == 2)) {
            printf("Win! Enemy loses %d HP.\n", player_damage);
            enemy_hp -= player_damage;

        } else {
            printf("Lose! You lose %d HP.\n", enemy_damage);
            player.hp -= enemy_damage;
        }
        printf("Enemy HP: %d | Player HP: %d\n", enemy_hp, player.hp);
    }
    if (player.hp <= 0) {
        printf("You are dead...\n");
        exit(0);
    } else {
        printf("Victory!\n");
        if (s->has_azatoth) {
            printf("\nBRAVO! You have defeated Azatoth and saved the region!\n");
            exit(0);
        } else {
            // Classic enemy loot
            int loot = rand() % 3; // 0: Nothing, 1: Key, 2: HP
            if (loot == 1) {
                printf("The enemy dropped a key!\n");
                player.keys++;
            } else if (loot == 2) {
                printf("You recover some health! +10 HP.\n");
                player.hp += 10;
                if (player.hp > 100) player.hp = 100;
            } else {
                printf("The enemy had nothing on them.\n");
            }
            printf("\nPress Enter to continue...");
            getchar(); getchar();
        }
        s->has_enemy = 0; // Enemy defeated
    }
}

// Handles player movement
void move_player(char direction) {
    int dx = 0, dy = 0;
    
    switch(direction) {
        case 'Z': case 'z': dy = -1; break;
        case 'S': case 's': dy = 1; break;
        case 'Q': case 'q': dx = -1; break;
        case 'D': case 'd': dx = 1; break;
        default: return;
    }
    
    int new_x = player.x + dx;
    int new_y = player.y + dy;
    
    Room* s = &map[player.current_room];

    // Check collision with outer walls
    int is_border = (new_x == 0 || new_x == WIDTH-1 || new_y == 0 || new_y == HEIGHT-1);
    
    // Indicates if the position corresponds to a door
    int is_door_n = (new_x == 5 && new_y == 0);
    int is_door_s = (new_x == 5 && new_y == HEIGHT-1);
    int is_door_w = (new_x == 0 && new_y == 5);
    int is_door_e = (new_x == WIDTH-1 && new_y == 5);
    
    int is_a_door = (is_door_n || is_door_s || is_door_w || is_door_e);

    // If it's a wall but not a door, block
    if (is_border && !is_a_door) {
        return;
    }
    // Check collision with enemy
    if (new_x == s->enemy_x && new_y == s->enemy_y && (s->has_enemy || s->has_azatoth)) {
        combat();
    }

    // Door management
    if (is_a_door) {
        int door_direction = -1;

        if (is_door_n) door_direction = NORTH;
        else if (is_door_s) door_direction = SOUTH;
        else if (is_door_e) door_direction = EAST;
        else if (is_door_w) door_direction = WEST;
        
        // Checks if a door exists (not -1)
        if (s->doors[door_direction] != -1) {
            // Checks if the door is locked
            if (s->locked_doors[door_direction]) {
                if (player.keys > 0) { // If the player has a key
                    printf("Locked door 'L'. You use a key!\n");
                    player.keys--;
                    s->locked_doors[door_direction] = 0; // Unlocks the door
                    int dest_room = s->doors[door_direction]; // Identifier of the destination room
                    int opp_dir = (door_direction + 2) % 4; // gets the opposite door
                    map[dest_room].locked_doors[opp_dir] = 0; // Unlocks the door in the other room
                    printf("\nPress Enter to continue...");
                    getchar(); getchar();
                } else {
                    printf("Locked door 'L'. You need a key.\n");
                    printf("\nPress Enter to continue...");
                    getchar(); getchar();
                    return;
                }
            }
            
            // room change
            player.current_room = s->doors[door_direction];
            if (door_direction == NORTH) { player.x = WIDTH/2; player.y = HEIGHT - 2; }
            else if (door_direction == SOUTH) { player.x = WIDTH/2; player.y = 1; }
            else if (door_direction == EAST) { player.x = 1; player.y = HEIGHT/2; }
            else if (door_direction == WEST) { player.x = WIDTH - 2; player.y = HEIGHT/2; }
            printf("You enter room %d.\n", player.current_room);
            getchar(); getchar();
        }
        return;
    }
    
    // Normal movement
    player.x = new_x;
    player.y = new_y;

    // Pick up treasure
    if (s->has_treasure && player.x == s->treasure_x && player.y == s->treasure_y) {
        int treasure_type = rand() % 3; // 0: HP, 1: Strength, 2: Defense
        if (treasure_type == 0) {
            printf("You find a potion! +20 HP.\n");
            player.hp += 20;
            if (player.hp > 100) player.hp = 100;
        } else if (treasure_type == 1) {
            printf("You find a long sword! Strength +2.\n");
            player.strength += 2;
        } else {
            printf("You find a shield! Defense +1.\n");
            player.defense += 1;
        }
        s->has_treasure = 0;
        printf("\nPress Enter to continue...");
        getchar(); getchar();
    }
}

void save_game(const char* filename) {
    // Binary write ("wb")
    FILE* file = fopen(filename, "wb");
    if (!file) {
        printf("Error: impossible to create save file.\n");
        return;
    }
    
    // Writes the player structure to the file
    fwrite(&player, sizeof(Knight), 1, file);
    // Writes the complete map array to the file
    fwrite(map, sizeof(Room), MAX_ROOMS, file);
    
    fclose(file);
    printf("Game saved!\n");
}

int load_save(const char* filename) {
    // Binary read ("rb")
    FILE* file = fopen(filename, "rb");
    if (!file) {
        return 0;
    }
    
    // Reads the Knight structure from the file and checks for errors
    if (fread(&player, sizeof(Knight), 1, file) != 1) {
        fclose(file);
        return 0;
    }
    // Reads the Room array from the file and checks for errors
    if (fread(map, sizeof(Room), MAX_ROOMS, file) != MAX_ROOMS) {
        fclose(file);
        return 0;
    }
    
    fclose(file);
    return 1;
}

void generate_random_dungeon() {
    printf("Generating random dungeon...\n");
    initialize_map();
    
    int visited[MAX_ROOMS];
    for(int i=0; i<MAX_ROOMS; i++) visited[i] = 0;
    
    // Start at room 1
    int current_rooms = 0;
    int start_room = 1;
    visited[start_room] = 1;
    current_rooms++;
    
    // Connect between 8 and MAX_ROOMS-1 rooms
    int target_rooms = 8 + (rand() % (MAX_ROOMS - 9)); 
    
    while (current_rooms < target_rooms) {
        // Choose an already visited room at random
        int r1 = -1;
        do {
            r1 = rand() % MAX_ROOMS;
        } while (!visited[r1]);
        
        // Choose a random direction
        int dir = rand() % 4; // 0-3
        
        // If the door is free
        if (map[r1].doors[dir] == -1) {
            // Find an unvisited room
            int r2 = -1;
            // Search for a free room
            for (int k = 1; k < MAX_ROOMS; k++) {
                if (!visited[k]) {
                    r2 = k;
                    break;
                }
            }
            
            if (r2 != -1) {
                // Connection
                int opp = (dir + 2) % 4;
                map[r1].doors[dir] = r2;
                map[r2].doors[opp] = r1;
                
                visited[r2] = 1;
                current_rooms++;
                
                // Generate content
                generate_room_content(r1);
                generate_room_content(r2);
                
                // Chance to lock (10%)
                if (rand() % 10 == 0) {
                    map[r1].locked_doors[dir] = 1;
                    map[r2].locked_doors[opp] = 1;
                }
            }
        }
    }
    
    // Place Azatoth in the last added room or a distant room
    // To simplify, take a random visited room that is not 1
    int boss_room = -1;
    while (boss_room == -1 || boss_room == 1) {
        int r = rand() % MAX_ROOMS;
        if (visited[r]) boss_room = r;
    }
    
    // Clean boss room
    map[boss_room].has_enemy = 0;
    map[boss_room].has_treasure = 0;
    map[boss_room].has_azatoth = 1;
    generate_room_content(boss_room); // Re-place boss correctly
    
    printf("Dungeon generated! %d rooms connected. Boss in room %d.\n", current_rooms, boss_room);
}

int main() {
    srand(time(NULL));
    
    // Initialization
    initialize_map();
    initialize_player();
    
    printf("Welcome, Knight. Find Azatoth!\n");
    printf("Load a save game? (Y/N): ");
    char choice;
    scanf(" %c", &choice);
    int game_loaded = 0;
    
    if (choice == 'Y' || choice == 'y' || choice == 'O' || choice == 'o') {
        if (load_save("save.sav")) {
            printf("Save loaded!\n");
            game_loaded = 1;
        } else {
            printf("No save found.\n");
        }
    }
    
    if (!game_loaded) {
        printf("Do you want a random dungeon? (Y/N): ");
        char dungeon_choice;
        scanf(" %c", &dungeon_choice);
        if (dungeon_choice == 'Y' || dungeon_choice == 'y' || dungeon_choice == 'O' || dungeon_choice == 'o') {
            generate_random_dungeon();
        } else {
            load_map("plan_base.pln");
        }
    }

    printf("Press Enter to start...");
    getchar(); getchar();

    // Game loop
    char command;
    while(1) {
        system("clear");
        display_status();
        display_room();
        
        printf("-> ");
        scanf(" %c", &command);
        
        if (command == 'X' || command == 'x') {
            break;
        } else if (command == 'P' || command == 'p') {
            save_game("save.sav");
            printf("\nPress Enter to continue...");
            getchar(); getchar();
        } else {
            move_player(command);
            move_enemies();
        }
    }
    
    return 0;
}
