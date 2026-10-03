#ifndef BUFFER_H
#define BUFFER_H

#include <stddef.h>

#include "writer.h"

struct buffer {
    char *data;
    size_t len;
    size_t cap;
};

void buffer_init(struct buffer *b);
void buffer_free(struct buffer *b);
struct writer buffer_writer(struct buffer *b);

#endif /* BUFFER_H */
