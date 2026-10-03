#ifndef GENERIC_ARRAY_H
#define GENERIC_ARRAY_H

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#define DEFINE_ARRAY(NAME, TYPE)                                               \
    struct NAME {                                                              \
        TYPE *data;                                                            \
        size_t len;                                                            \
        size_t cap;                                                            \
    };                                                                         \
                                                                               \
    static inline void NAME##_init(struct NAME *a)                             \
    {                                                                          \
        a->data = NULL;                                                        \
        a->len = 0;                                                            \
        a->cap = 0;                                                            \
    }                                                                          \
                                                                               \
    static inline void NAME##_free(struct NAME *a)                             \
    {                                                                          \
        free(a->data);                                                         \
        a->data = NULL;                                                        \
        a->len = 0;                                                            \
        a->cap = 0;                                                            \
    }                                                                          \
                                                                               \
    static inline bool NAME##_push(struct NAME *a, TYPE value)                 \
    {                                                                          \
        if (a->len == a->cap) {                                                \
            size_t cap = a->cap == 0 ? 4 : a->cap * 2;                         \
            TYPE *data = realloc(a->data, cap * sizeof *data);                 \
            if (data == NULL) {                                                \
                return false;                                                  \
            }                                                                  \
            a->data = data;                                                    \
            a->cap = cap;                                                      \
        }                                                                      \
        a->data[a->len] = value;                                               \
        a->len++;                                                              \
        return true;                                                           \
    }

#endif /* GENERIC_ARRAY_H */
