#include "buffer.h"

#include <stdlib.h>
#include <string.h>

static bool buffer_grow(struct buffer *b, size_t needed)
{
    if (needed + 1 <= b->cap) {
        return true;
    }

    size_t cap = b->cap == 0 ? 16 : b->cap;
    while (cap < needed + 1) {
        cap *= 2;
    }

    char *data = realloc(b->data, cap);
    if (data == NULL) {
        return false;
    }

    b->data = data;
    b->cap = cap;
    return true;
}

static int buffer_write(void *ctx, const char *s, size_t n)
{
    struct buffer *b = ctx;

    if (!buffer_grow(b, b->len + n)) {
        return -1;
    }

    memcpy(b->data + b->len, s, n);
    b->len += n;
    b->data[b->len] = '\0';
    return (int)n;
}

void buffer_init(struct buffer *b)
{
    b->data = NULL;
    b->len = 0;
    b->cap = 0;

    if (buffer_grow(b, 0)) {
        b->data[0] = '\0';
    }
}

void buffer_free(struct buffer *b)
{
    free(b->data);
    b->data = NULL;
    b->len = 0;
    b->cap = 0;
}

struct writer buffer_writer(struct buffer *b)
{
    return (struct writer){.write = buffer_write, .ctx = b};
}
