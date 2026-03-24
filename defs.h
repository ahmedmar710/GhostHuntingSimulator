#ifndef DEFS_H
#define DEFS_H

#include <stdbool.h>
#include <semaphore.h>
#include <pthread.h>

#define MAX_ROOM_NAME 64
#define MAX_HUNTER_NAME 64
#define MAX_ROOMS 24
#define MAX_ROOM_OCCUPANCY 8
#define MAX_CONNECTIONS 8
#define ENTITY_BOREDOM_MAX 15
#define HUNTER_FEAR_MAX 15
#define DEFAULT_GHOST_ID 68057

typedef unsigned char EvidenceByte;

enum LogReason {
    LR_EVIDENCE = 0,
    LR_BORED = 1,
    LR_AFRAID = 2
};

enum EvidenceType {
    EV_EMF          = 1 << 0,
    EV_ORBS         = 1 << 1,
    EV_RADIO        = 1 << 2,
    EV_TEMPERATURE  = 1 << 3,
    EV_FINGERPRINTS = 1 << 4,
    EV_WRITING      = 1 << 5,
    EV_INFRARED     = 1 << 6,
};

enum GhostType {
    GH_POLTERGEIST  = EV_FINGERPRINTS | EV_TEMPERATURE | EV_WRITING,
    GH_THE_MIMIC    = EV_FINGERPRINTS | EV_TEMPERATURE | EV_RADIO,
    GH_HANTU        = EV_FINGERPRINTS | EV_TEMPERATURE | EV_ORBS,
    GH_JINN         = EV_FINGERPRINTS | EV_TEMPERATURE | EV_EMF,
    GH_PHANTOM      = EV_FINGERPRINTS | EV_INFRARED    | EV_RADIO,
    GH_BANSHEE      = EV_FINGERPRINTS | EV_INFRARED    | EV_ORBS,
    GH_GORYO        = EV_FINGERPRINTS | EV_INFRARED    | EV_EMF,
    GH_BULLIES      = EV_FINGERPRINTS | EV_WRITING     | EV_RADIO,
    GH_MYLING       = EV_FINGERPRINTS | EV_WRITING     | EV_EMF,
    GH_OBAKE        = EV_FINGERPRINTS | EV_ORBS        | EV_EMF,
    GH_YUREI        = EV_TEMPERATURE  | EV_INFRARED    | EV_ORBS,
    GH_ONI          = EV_TEMPERATURE  | EV_INFRARED    | EV_EMF,
    GH_MOROI        = EV_TEMPERATURE  | EV_WRITING     | EV_RADIO,
    GH_REVENANT     = EV_TEMPERATURE  | EV_WRITING     | EV_ORBS,
    GH_SHADE        = EV_TEMPERATURE  | EV_WRITING     | EV_EMF,
    GH_ONRYO        = EV_TEMPERATURE  | EV_RADIO       | EV_ORBS,
    GH_THE_TWINS    = EV_TEMPERATURE  | EV_RADIO       | EV_EMF,
    GH_DEOGEN       = EV_INFRARED     | EV_WRITING     | EV_RADIO,
    GH_THAYE        = EV_INFRARED     | EV_WRITING     | EV_ORBS,
    GH_YOKAI        = EV_INFRARED     | EV_RADIO       | EV_ORBS,
    GH_WRAITH       = EV_INFRARED     | EV_RADIO       | EV_EMF,
    GH_RAIJU        = EV_INFRARED     | EV_ORBS        | EV_EMF,
    GH_MARE         = EV_WRITING      | EV_RADIO       | EV_ORBS,
    GH_SPIRIT       = EV_WRITING      | EV_RADIO       | EV_EMF,
};

struct CaseFile {
    EvidenceByte collected;
    bool         solved;
    sem_t        mutex;
};

struct RoomNode {
    struct Room* room;
    struct RoomNode* next;
};

struct RoomStack {
    struct RoomNode* head;
};

struct Ghost;
struct Hunter;

struct Room {
    char name[MAX_ROOM_NAME];
    struct Room* connections[MAX_CONNECTIONS];
    int connection_count;
    struct Ghost* ghost;
    struct Hunter* hunters[MAX_ROOM_OCCUPANCY];
    int hunter_count;
    bool is_exit;
    EvidenceByte evidence;
    sem_t mutex;
};

struct Ghost {
    int id;
    enum GhostType type;
    struct Room* current_room;
    int boredom;
    bool has_exited;
};

struct Hunter {
    char name[MAX_HUNTER_NAME];
    int id;
    struct Room* current_room;
    struct CaseFile* case_file;
    enum EvidenceType device;
    struct RoomStack path;
    int fear;
    int boredom;
    enum LogReason exit_reason;
    bool has_exited;
    bool returning_to_van;
};

struct HunterArray {
    struct Hunter* hunters;
    int count;
    int capacity;
};

struct House {
    struct Room rooms[MAX_ROOMS];
    int room_count;
    struct Room* starting_room;
    struct HunterArray hunter_array;
    struct CaseFile case_file;
    struct Ghost ghost;
};

void room_init(struct Room* room, const char* name, bool is_exit);
void room_connect(struct Room* a, struct Room* b);

#endif // DEFS_H
