#ifndef INTS_H
#define INTS_H

#include <stdbool.h>
#include <stddef.h>

struct ints {
    int *data;
    size_t len;
    size_t cap;
};

/* Create an empty, heap-allocated list. */
struct ints ints_new(void);

/* Release the memory owned by the list. */
void ints_free(struct ints *xs);

/* Append a value, growing if necessary. Returns false on allocation failure. */
bool ints_append(struct ints *xs, int value);

#endif /* INTS_H */
