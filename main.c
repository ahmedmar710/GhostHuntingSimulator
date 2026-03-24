// main.c - Main control flow
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "defs.h"
#include "helpers.h"

extern void house_init(struct House* house);
extern void house_cleanup(struct House* house);
extern void hunter_array_init(struct HunterArray* arr);
extern void hunter_array_append(struct HunterArray* arr, struct Hunter* hunter);
extern void hunter_init(struct Hunter* hunter, int id, const char* name, 
                        struct Room* start_room, struct CaseFile* case_file);
extern void ghost_init(struct Ghost* ghost, struct House* house);
extern void* hunter_thread(void* arg);
extern void* ghost_thread(void* arg);
extern bool room_add_hunter(struct Room* room, struct Hunter* hunter);

void print_results(struct House* house) {
    printf("\n========== SIMULATION RESULTS ==========\n\n");
    
    printf("Ghost Type: %s\n", ghost_to_string(house->ghost.type));
    printf("Ghost Evidence: ");
    
    int ghost_count = 0;
    for (int i = 0; i < 7; i++) {
        enum EvidenceType type = (1 << i);
        if ((EvidenceByte)house->ghost.type & type) {
            ghost_count++;
            printf("%s ", evidence_to_string(type));
        }
    }
    printf("\n\n");
    
    printf("Collected Evidence: ");
    if (house->case_file.collected == 0) {
        printf("None\n");
    } else {
        for (int i = 0; i < 7; i++) {
            enum EvidenceType type = (1 << i);
            if (house->case_file.collected & type) {
                printf("%s ", evidence_to_string(type));
            }
        }
        printf("\n");
    }
    
    if (evidence_is_valid_ghost(house->case_file.collected)) {
        printf("Identified Ghost: %s\n", ghost_to_string((enum GhostType)house->case_file.collected));
    } else {
        printf("Identified Ghost: Unknown (not enough evidence)\n");
    }
    
    printf("\n");
    
    printf("Hunter Exit Reasons:\n");
    for (int i = 0; i < house->hunter_array.count; i++) {
        struct Hunter* hunter = &house->hunter_array.hunters[i];
        printf("  %s (ID %d): %s\n", 
               hunter->name, 
               hunter->id, 
               exit_reason_to_string(hunter->exit_reason));
    }
    
    printf("\n========================================\n");
}

int main() {
    struct House house;
    house_init(&house);
    house_populate_rooms(&house);
    
    hunter_array_init(&house.hunter_array);
    
    printf("Ghost Hunt Simulation\n");
    printf("Enter hunter names and IDs. Type 'done' when finished.\n\n");
    
    while (1) {
        char name[MAX_HUNTER_NAME];
        int id;
        
        printf("Enter hunter name (or 'done'): ");
        if (scanf("%s", name) != 1) {
            break;
        }
        
        if (strcmp(name, "done") == 0) {
            break;
        }
        
        printf("Enter hunter ID: ");
        if (scanf("%d", &id) != 1) {
            break;
        }
        
        struct Hunter hunter;
        hunter_init(&hunter, id, name, house.starting_room, &house.case_file);
        hunter_array_append(&house.hunter_array, &hunter);
    }
    
    if (house.hunter_array.count == 0) {
        printf("No hunters entered. Exiting.\n");
        house_cleanup(&house);
        return 0;
    }
    
    printf("\nStarting simulation with %d hunter(s)...\n\n", house.hunter_array.count);
    
    ghost_init(&house.ghost, &house);
    
    pthread_t ghost_tid;
    pthread_create(&ghost_tid, NULL, ghost_thread, &house.ghost);
    
    pthread_t* hunter_tids = malloc(house.hunter_array.count * sizeof(pthread_t));
    for (int i = 0; i < house.hunter_array.count; i++) {
        pthread_create(&hunter_tids[i], NULL, hunter_thread, &house.hunter_array.hunters[i]);
    }
    
    pthread_join(ghost_tid, NULL);
    
    for (int i = 0; i < house.hunter_array.count; i++) {
        pthread_join(hunter_tids[i], NULL);
    }
    
    free(hunter_tids);
    
    print_results(&house);
    
    house_cleanup(&house);
    
    return 0;
}
