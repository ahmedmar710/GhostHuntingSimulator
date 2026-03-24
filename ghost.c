// ghost.c - Ghost behavior and logic
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "defs.h"
#include "helpers.h"

extern void evidence_get_types(EvidenceByte source, enum EvidenceType* types, int* count);
extern void room_add_evidence(struct Room* room, enum EvidenceType evidence);

void ghost_init(struct Ghost* ghost, struct House* house) {
    ghost->id = DEFAULT_GHOST_ID;
    ghost->boredom = 0;
    ghost->has_exited = false;
    
    const enum GhostType* ghost_types;
    int type_count = get_all_ghost_types(&ghost_types);
    int type_idx = rand_int_threadsafe(0, type_count);
    ghost->type = ghost_types[type_idx];
    
    int room_idx = rand_int_threadsafe(0, house->room_count);
    ghost->current_room = &house->rooms[room_idx];
    ghost->current_room->ghost = ghost;
    
    log_ghost_init(ghost->id, ghost->current_room->name, ghost->type);
}

void ghost_update_stats(struct Ghost* ghost) {
    if (ghost->current_room->hunter_count > 0) {
        ghost->boredom = 0;
    } else {
        ghost->boredom++;
    }
}

void ghost_leave_evidence(struct Ghost* ghost) {
    enum EvidenceType evidence_types[3];
    int count = 0;
    
    evidence_get_types((EvidenceByte)ghost->type, evidence_types, &count);
    
    if (count > 0) {
        int idx = rand_int_threadsafe(0, count);
        room_add_evidence(ghost->current_room, evidence_types[idx]);
        log_ghost_evidence(ghost->id, ghost->boredom, 
                          ghost->current_room->name, evidence_types[idx]);
    }
}

void ghost_move(struct Ghost* ghost) {
    if (ghost->current_room->hunter_count > 0) {
        return;
    }
    
    if (ghost->current_room->connection_count == 0) {
        return;
    }
    
    int idx = rand_int_threadsafe(0, ghost->current_room->connection_count);
    struct Room* target = ghost->current_room->connections[idx];
    
    struct Room* current = ghost->current_room;
    struct Room* first = (current < target) ? current : target;
    struct Room* second = (current < target) ? target : current;
    
    sem_wait(&first->mutex);
    sem_wait(&second->mutex);
    
    current->ghost = NULL;
    target->ghost = ghost;
    
    log_ghost_move(ghost->id, ghost->boredom, current->name, target->name);
    
    ghost->current_room = target;
    
    sem_post(&second->mutex);
    sem_post(&first->mutex);
}

void ghost_take_turn(struct Ghost* ghost) {
    if (ghost->has_exited) {
        return;
    }
    
    ghost_update_stats(ghost);
    
    if (ghost->boredom > ENTITY_BOREDOM_MAX) {
        ghost->current_room->ghost = NULL;
        ghost->has_exited = true;
        log_ghost_exit(ghost->id, ghost->boredom, ghost->current_room->name);
        return;
    }
    
    int action = rand_int_threadsafe(0, 3);
    
    if (action == 0) {
        log_ghost_idle(ghost->id, ghost->boredom, ghost->current_room->name);
    } else if (action == 1) {
        ghost_leave_evidence(ghost);
    } else {
        ghost_move(ghost);
    }
}

void* ghost_thread(void* arg) {
    struct Ghost* ghost = (struct Ghost*)arg;
    
    while (!ghost->has_exited) {
        ghost_take_turn(ghost);
        usleep(1000);
    }
    
    return NULL;
}
