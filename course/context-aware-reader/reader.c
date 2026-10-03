#include "reader.h"

#include <errno.h>
#include <poll.h>
#include <unistd.h>

void read_context_init(struct read_context *ctx)
{
    atomic_init(&ctx->cancelled, false);
}

void read_context_cancel(struct read_context *ctx)
{
    atomic_store(&ctx->cancelled, true);
}

bool read_context_done(const struct read_context *ctx)
{
    return atomic_load(&ctx->cancelled);
}

int read_with_context(int fd, const struct read_context *ctx, char *buf,
                      size_t cap, int timeout_ms)
{
    int waited = 0;
    bool forever = timeout_ms < 0;

    while (forever || waited < timeout_ms) {
        struct pollfd pfd = {.fd = fd, .events = POLLIN, .revents = 0};
        int step = 10;
        int ready;

        if (read_context_done(ctx)) {
            return -1;
        }

        if (!forever && timeout_ms - waited < step) {
            step = timeout_ms - waited;
        }

        ready = poll(&pfd, 1, step);
        if (ready < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        if (ready == 0) {
            waited += step;
            continue;
        }

        ssize_t n = read(fd, buf, cap);
        if (n < 0) {
            return -1;
        }
        return (int)n;
    }

    return 0; /* timed out */
}

int copy_with_context(int in_fd, int out_fd, const struct read_context *ctx,
                      int timeout_ms)
{
    int total = 0;
    char buffer[256];

    for (;;) {
        int n =
            read_with_context(in_fd, ctx, buffer, sizeof buffer, timeout_ms);

        if (n < 0) {
            return -1; /* cancelled or error */
        }
        if (n == 0) {
            return total; /* timeout: stop for now */
        }
        if (write(out_fd, buffer, (size_t)n) != n) {
            return -1;
        }
        total += n;
    }
}
