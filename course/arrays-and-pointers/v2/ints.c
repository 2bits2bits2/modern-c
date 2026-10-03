#include "ints.h"

#include <stdlib.h>
#include <string.h>

struct ints ints_new(void)
{
    return (struct ints){.data = NULL, .len = 0, .cap = 0};
}

void ints_free(struct ints *xs)
{
    free(xs->data);
    xs->data = NULL;
    xs->len = 0;
    xs->cap = 0;
}

static bool ints_grow(struct ints *xs, size_t needed)
{
    if (needed <= xs->cap) {
        return true;
    }

    size_t new_cap = xs->cap == 0 ? 4 : xs->cap * 2;
    while (new_cap < needed) {
        new_cap *= 2;
    }

    int *new_data = realloc(xs->data, new_cap * sizeof *new_data);
    if (new_data == NULL) {
        return false;
    }

    xs->data = new_data;
    xs->cap = new_cap;
    return true;
}

bool ints_append(struct ints *xs, int value)
{
    if (!ints_grow(xs, xs->len + 1)) {
        return false;
    }

    xs->data[xs->len] = value;
    xs->len++;
    return true;
}
