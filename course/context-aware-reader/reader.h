#ifndef READER_H
#define READER_H

#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>

/* A cancellable read context, shared with whichever thread may cancel it. */
struct read_context {
    atomic_bool cancelled;
};

void read_context_init(struct read_context *ctx);
void read_context_cancel(struct read_context *ctx);
bool read_context_done(const struct read_context *ctx);

/*
 * Read once from fd, waiting up to timeout_ms for data (pass -1 to wait
 * forever). Returns the number of bytes read, 0 on timeout, or -1 if the
 * context was cancelled or an error occurred.
 */
int read_with_context(int fd, const struct read_context *ctx, char *buf,
                      size_t cap, int timeout_ms);

/*
 * Copy from in_fd to out_fd until EOF, a timeout, or cancellation. Returns the
 * number of bytes copied, or -1 if cancelled or on error.
 */
int copy_with_context(int in_fd, int out_fd, const struct read_context *ctx,
                      int timeout_ms);

#endif /* READER_H */
