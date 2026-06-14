#ifndef ID_SET_HEADER
#define ID_SET_HEADER

#include <stdbool.h>
#include <stddef.h>

/** A hash-set of identifier strings (owns copies of the keys). */
typedef struct IdSet IdSet;

IdSet * createIdSet(void);
void destroyIdSet(IdSet * set);

/** Inserts "id". Returns true if inserted, false if it was already present. */
bool idSetAdd(IdSet * set, const char * id);

/** Returns true if "id" is present. */
bool idSetContains(const IdSet * set, const char * id);

#endif
