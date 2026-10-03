#include "collections.h"

int reduce_sum(const struct int_array *a)
{
    int total = 0;

    for (size_t i = 0; i < a->len; i++) {
        total += a->data[i];
    }

    return total;
}

struct int_array map_double_all(const struct int_array *a)
{
    struct int_array out;

    int_array_init(&out);
    for (size_t i = 0; i < a->len; i++) {
        int_array_push(&out, a->data[i] * 2);
    }

    return out;
}

struct int_array filter_even(const struct int_array *a)
{
    struct int_array out;

    int_array_init(&out);
    for (size_t i = 0; i < a->len; i++) {
        if (a->data[i] % 2 == 0) {
            int_array_push(&out, a->data[i]);
        }
    }

    return out;
}
