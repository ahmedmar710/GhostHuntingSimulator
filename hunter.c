// hunter.c - Hunter behavior and logic
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "defs.h"
#include "helpers.h"

extern void evidence_set(EvidenceByte* evidence, enum EvidenceType type);
extern bool evidence_has(EvidenceByte evidence, enum EvidenceType type);
extern int evidence_count_bits(EvidenceByte evidence);
extern void room_add_evidence(struct Room* room, enum EvidenceType evidence);
extern void room_remove_evidence(struct Room* room, enum EvidenceType evidence);
extern bool room_has_evidence(struct Room* room, enum EvidenceType evidence);
extern bool room_add_hunter(struct Room* room, struct Hunter* hunter);
extern void room_remove_hunter(struct Room* room, struct Hunter* hunter);
extern void stack_init(struct RoomStack* stack);
extern void stack_push(struct RoomStack* stack, struct Room* room);
extern struct Room* stack_pop(struct RoomStack* stack);
extern void stack_clear(struct RoomStack* stack);
extern bool stack_is_empty(struct RoomStack* stack);

void hunter_init(struct Hunter* hunter, int id, const char* name, 
                 struct Room* start_room, struct CaseFile* case_file) {
    strncpy(hunter->name, name, MAX_HUNTER_NAME - 1);
    hunter->name[MAX_HUNTER_NAME - 1] = '\0';
    
    hunter->id = id;
    hunter->current_room = start_room;
    hunter->case_file = case_file;
    hunter->fear = 0;
    hunter->boredom = 0;
    hunter->has_exited = false;
    hunter->returning_to_van = false;
    hunter->exit_reason = LR_BORED;
    
    const enum EvidenceType* all_evidence;
    int count = get_all_evidence_types(&all_evidence);
    int idx = rand_int_threadsafe(0, count);
    hunter->device = all_evidence[idx];
    
    stack_init(&hunter->path);
    
    log_hunter_init(id, start_room->name, name, hunter->device);
}

void hunter_update_stats(struct Hunter* hunter) {
    if (hunter->current_room->ghost != NULL) {
        hunter->boredom = 0;
        hunter->fear++;
    } else {
        hunter->boredom++;
    }
}

void hunter_gather_evidence(struct Hunter* hunter) {
    struct Room* room = hunter->current_room;
    
    if (room_has_evidence(room, hunter->device)) {
        room_remove_evidence(room, hunter->device);
        
        sem_wait(&hunter->case_file->mutex);
        evidence_set(&hunter->case_file->collected, hunter->device);
        
        if (evidence_count_bits(hunter->case_file->collected) >= 3 &&
            evidence_is_valid_ghost(hunter->case_file->collected)) {
            hunter->case_file->solved = true;
        }
        sem_post(&hunter->case_file->mutex);
        
        log_evidence(hunter->id, hunter->boredom, hunter->fear, 
                    room->name, hunter->device);
        
        if (!room->is_exit) {
            hunter->returning_to_van = true;
            log_return_to_van(hunter->id, hunter->boredom, hunter->fear,
                            room->name, hunter->device, true);
        }
    } else {
        if (rand_int_threadsafe(0, 100) < 10) {
            if (!room->is_exit && !hunter->returning_to_van) {
                hunter->returning_to_van = true;
                log_return_to_van(hunter->id, hunter->boredom, hunter->fear,
                                room->name, hunter->device, true);
            }
        }
    }
}

bool hunter_attempt_move(struct Hunter* hunter, struct Room* target) {
    if (target == NULL) {
        return false;
    }
    
    struct Room* current = hunter->current_room;
    
    struct Room* first = (current < target) ? current : target;
    struct Room* second = (current < target) ? target : current;
    
    sem_wait(&first->mutex);
    sem_wait(&second->mutex);
    
    bool success = false;
    if (target->hunter_count < MAX_ROOM_OCCUPANCY) {
        room_remove_hunter(current, hunter);
        room_add_hunter(target, hunter);
        
        log_move(hunter->id, hunter->boredom, hunter->fear,
                current->name, target->name, hunter->device);
        
        hunter->current_room = target;
        success = true;
    }
    
    sem_post(&second->mutex);
    sem_post(&first->mutex);
    
    return success;
}

void hunter_move(struct Hunter* hunter) {
    struct Room* target = NULL;
    
    if (hunter->returning_to_van) {
        target = stack_pop(&hunter->path);
        
        if (target != NULL) {
            hunter_attempt_move(hunter, target);
        }
        
        if (hunter->current_room->is_exit) {
            stack_clear(&hunter->path);
            hunter->returning_to_van = false;
            log_return_to_van(hunter->id, hunter->boredom, hunter->fear,
                            hunter->current_room->name, hunter->device, false);
        }
    } else {
        if (hunter->current_room->connection_count > 0) {
            int idx = rand_int_threadsafe(0, hunter->current_room->connection_count);
            target = hunter->current_room->connections[idx];
            
            struct Room* old_room = hunter->current_room;
            if (hunter_attempt_move(hunter, target)) {
                stack_push(&hunter->path, old_room);
            }
        }
    }
}

void hunter_swap_device(struct Hunter* hunter) {
    enum EvidenceType old_device = hunter->device;
    
    const enum EvidenceType* all_evidence;
    int count = get_all_evidence_types(&all_evidence);
    int idx = rand_int_threadsafe(0, count);
    hunter->device = all_evidence[idx];
    
    log_swap(hunter->id, hunter->boredom, hunter->fear, old_device, hunter->device);
}

void hunter_take_turn(struct Hunter* hunter) {
    if (hunter->has_exited) {
        return;
    }
    
    hunter_update_stats(hunter);
    
    if (hunter->current_room->is_exit) {
        stack_clear(&hunter->path);
        
        sem_wait(&hunter->case_file->mutex);
        bool solved = hunter->case_file->solved;
        sem_post(&hunter->case_file->mutex);
        
        if (solved) {
            room_remove_hunter(hunter->current_room, hunter);
            hunter->has_exited = true;
            hunter->exit_reason = LR_EVIDENCE;
            log_exit(hunter->id, hunter->boredom, hunter->fear,
                    hunter->current_room->name, hunter->device, LR_EVIDENCE);
            return;
        }
        
        hunter_swap_device(hunter);
    }
    
    if (hunter->boredom > ENTITY_BOREDOM_MAX) {
        room_remove_hunter(hunter->current_room, hunter);
        hunter->has_exited = true;
        hunter->exit_reason = LR_BORED;
        log_exit(hunter->id, hunter->boredom, hunter->fear,
                hunter->current_room->name, hunter->device, LR_BORED);
        return;
    }
    
    if (hunter->fear >= HUNTER_FEAR_MAX) {
        room_remove_hunter(hunter->current_room, hunter);
        hunter->has_exited = true;
        hunter->exit_reason = LR_AFRAID;
        log_exit(hunter->id, hunter->boredom, hunter->fear,
                hunter->current_room->name, hunter->device, LR_AFRAID);
        return;
    }
    
    hunter_gather_evidence(hunter);
    hunter_move(hunter);
}

void* hunter_thread(void* arg) {
    struct Hunter* hunter = (struct Hunter*)arg;
    
    while (!hunter->has_exited) {
        hunter_take_turn(hunter);
        usleep(1000);
    }
    
    return NULL;
}

void hunter_cleanup(struct Hunter* hunter) {
    stack_clear(&hunter->path);
}
