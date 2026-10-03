#ifndef WRITER_H
#define WRITER_H

#include <stddef.h>

struct writer {
    int (*write)(void *ctx, const char *s, size_t n);
    void *ctx;
};

int writer_write(struct writer w, const char *s);

#endif /* WRITER_H */
