// evidence.c - Evidence bitwise operations
#include "defs.h"

void evidence_set(EvidenceByte* evidence, enum EvidenceType type) {
    *evidence |= type;
}

void evidence_clear(EvidenceByte* evidence, enum EvidenceType type) {
    *evidence &= ~type;
}

bool evidence_has(EvidenceByte evidence, enum EvidenceType type) {
    return (evidence & type) != 0;
}

int evidence_count_bits(EvidenceByte evidence) {
    int count = 0;
    for (int i = 0; i < 7; i++) {
        if (evidence & (1 << i)) {
            count++;
        }
    }
    return count;
}

void evidence_get_types(EvidenceByte source, enum EvidenceType* types, int* count) {
    *count = 0;
    for (int i = 0; i < 7; i++) {
        enum EvidenceType type = (1 << i);
        if (source & type) {
            types[*count] = type;
            (*count)++;
        }
    }
}
