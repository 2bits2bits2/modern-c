#ifndef WRITER_H
#define WRITER_H

#include <stddef.h>

/*
 * A writer is anything that can accept a chunk of bytes. It is our io.Writer.
 * The function pointer gets a context, so a writer can be a buffer, a file, a
 * socket, or a test spy.
 */
struct writer {
    int (*write)(void *ctx, const char *s, size_t n);
    void *ctx;
};

int writer_write(struct writer w, const char *s);

#endif /* WRITER_H */
