// room.c - Room initialization and management
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "defs.h"
#include "helpers.h"

void room_init(struct Room* room, const char* name, bool is_exit) {
    strncpy(room->name, name, MAX_ROOM_NAME - 1);
    room->name[MAX_ROOM_NAME - 1] = '\0';
    
    room->connection_count = 0;
    room->hunter_count = 0;
    room->ghost = NULL;
    room->is_exit = is_exit;
    room->evidence = 0;
    
    sem_init(&room->mutex, 0, 1);
}

void room_connect(struct Room* a, struct Room* b) {
    if (a->connection_count < MAX_CONNECTIONS) {
        a->connections[a->connection_count] = b;
        a->connection_count++;
    }
    
    if (b->connection_count < MAX_CONNECTIONS) {
        b->connections[b->connection_count] = a;
        b->connection_count++;
    }
}

bool room_add_hunter(struct Room* room, struct Hunter* hunter) {
    if (room->hunter_count >= MAX_ROOM_OCCUPANCY) {
        return false;
    }
    
    room->hunters[room->hunter_count] = hunter;
    room->hunter_count++;
    return true;
}

void room_remove_hunter(struct Room* room, struct Hunter* hunter) {
    for (int i = 0; i < room->hunter_count; i++) {
        if (room->hunters[i] == hunter) {
            for (int j = i; j < room->hunter_count - 1; j++) {
                room->hunters[j] = room->hunters[j + 1];
            }
            room->hunter_count--;
            return;
        }
    }
}

void room_add_evidence(struct Room* room, enum EvidenceType evidence) {
    room->evidence |= evidence;
}

void room_remove_evidence(struct Room* room, enum EvidenceType evidence) {
    room->evidence &= ~evidence;
}

bool room_has_evidence(struct Room* room, enum EvidenceType evidence) {
    return (room->evidence & evidence) != 0;
}

void room_cleanup(struct Room* room) {
    sem_destroy(&room->mutex);
}

void stack_init(struct RoomStack* stack) {
    stack->head = NULL;
}

void stack_push(struct RoomStack* stack, struct Room* room) {
    struct RoomNode* node = malloc(sizeof(struct RoomNode));
    node->room = room;
    node->next = stack->head;
    stack->head = node;
}

struct Room* stack_pop(struct RoomStack* stack) {
    if (stack->head == NULL) {
        return NULL;
    }
    
    struct RoomNode* node = stack->head;
    struct Room* room = node->room;
    stack->head = node->next;
    free(node);
    
    return room;
}

void stack_clear(struct RoomStack* stack) {
    while (stack->head != NULL) {
        struct RoomNode* node = stack->head;
        stack->head = node->next;
        free(node);
    }
}

bool stack_is_empty(struct RoomStack* stack) {
    return stack->head == NULL;
}
