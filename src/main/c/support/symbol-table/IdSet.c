#include "IdSet.h"
#include <stdlib.h>
#include <string.h>

#define ID_SET_BUCKETS 97

typedef struct Entry {
	char * key;
	struct Entry * next;
} Entry;

struct IdSet {
	Entry * buckets[ID_SET_BUCKETS];
};

static size_t _hash(const char * s) {
	size_t h = 5381;
	for (; *s; ++s) {
		h = ((h << 5) + h) + (unsigned char) *s; /* djb2 */
	}
	return h % ID_SET_BUCKETS;
}

IdSet * createIdSet(void) {
	IdSet * set = calloc(1, sizeof(IdSet));
	return set;
}

void destroyIdSet(IdSet * set) {
	if (set == NULL) {
		return;
	}
	for (size_t i = 0; i < ID_SET_BUCKETS; ++i) {
		Entry * e = set->buckets[i];
		while (e != NULL) {
			Entry * next = e->next;
			free(e->key);
			free(e);
			e = next;
		}
	}
	free(set);
}

bool idSetContains(const IdSet * set, const char * id) {
	const Entry * e = set->buckets[_hash(id)];
	while (e != NULL) {
		if (strcmp(e->key, id) == 0) {
			return true;
		}
		e = e->next;
	}
	return false;
}

bool idSetAdd(IdSet * set, const char * id) {
	if (idSetContains(set, id)) {
		return false;
	}
	size_t b = _hash(id);
	Entry * e = malloc(sizeof(Entry));
	e->key = malloc(strlen(id) + 1);
	strcpy(e->key, id);
	e->next = set->buckets[b];
	set->buckets[b] = e;
	return true;
}

void idSetForEach(const IdSet * set, void (*visit)(const char * id, void * context), void * context) {
	for (size_t i = 0; i < ID_SET_BUCKETS; ++i) {
		for (const Entry * e = set->buckets[i]; e != NULL; e = e->next) {
			visit(e->key, context);
		}
	}
}
