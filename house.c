// house.c - House initialization and cleanup
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "helpers.h"

extern void room_cleanup(struct Room* room);
extern void hunter_cleanup(struct Hunter* hunter);

void house_init(struct House* house) {
    house->room_count = 0;
    house->starting_room = NULL;
    
    house->hunter_array.hunters = NULL;
    house->hunter_array.count = 0;
    house->hunter_array.capacity = 0;
    
    house->case_file.collected = 0;
    house->case_file.solved = false;
    sem_init(&house->case_file.mutex, 0, 1);
    
    house->ghost.id = 0;
    house->ghost.type = GH_POLTERGEIST;
    house->ghost.current_room = NULL;
    house->ghost.boredom = 0;
    house->ghost.has_exited = false;
}

void hunter_array_init(struct HunterArray* arr) {
    arr->capacity = 4;
    arr->count = 0;
    arr->hunters = malloc(arr->capacity * sizeof(struct Hunter));
}

void hunter_array_append(struct HunterArray* arr, struct Hunter* hunter) {
    if (arr->count >= arr->capacity) {
        arr->capacity *= 2;
        arr->hunters = realloc(arr->hunters, arr->capacity * sizeof(struct Hunter));
    }
    
    arr->hunters[arr->count] = *hunter;
    arr->count++;
}

void hunter_array_cleanup(struct HunterArray* arr) {
    for (int i = 0; i < arr->count; i++) {
        hunter_cleanup(&arr->hunters[i]);
    }
    
    if (arr->hunters != NULL) {
        free(arr->hunters);
        arr->hunters = NULL;
    }
    
    arr->count = 0;
    arr->capacity = 0;
}

void house_cleanup(struct House* house) {
    for (int i = 0; i < house->room_count; i++) {
        room_cleanup(&house->rooms[i]);
    }
    
    hunter_array_cleanup(&house->hunter_array);
    
    sem_destroy(&house->case_file.mutex);
}
