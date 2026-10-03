# A context-aware reader

**[You can find all the code for this chapter here](context-aware-reader/)**

A reader asked how to read from a connection but stop when asked — because the
server is shutting down, or the client has gone away. Go wraps an `io.Reader`
so that a context cancellation interrupts it. In C we build the same thing
from `poll` and a cancellable flag.

## Just enough

A plain `read` on a socket blocks forever if no data ever comes. To make it
cancellable we need two abilities:

- **wait with a timeout**, so we periodically get control back to check for
  cancellation — `poll` gives us this
- **a shared cancel flag**, safe to set from another thread — an `atomic_bool`

```c
struct read_context {
    atomic_bool cancelled;
};
```

This is the [context](context.md) chapter's idea, applied to reading. The
reader does not sleep for the full timeout; it waits in short steps, checking
the flag each step, so cancellation is prompt.

## Write the test first

```c
static void test_reads_available_data(void)
{
    int fds[2];
    struct read_context ctx;
    char buf[32];

    CHECK_INT(pipe(fds), 0);
    read_context_init(&ctx);
    CHECK_INT(write(fds[1], "hello", 5), 5);

    int n = read_with_context(fds[0], &ctx, buf, sizeof buf, 1000);
    CHECK_INT(n, 5);
    buf[n] = '\0';
    CHECK_STR(buf, "hello");
}
```

Pipes again: they are the perfect test double for "some file descriptor that
may or may not have data".

## Write enough code to make it pass

```c
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
        /* ... interpret ready ... */
    }

    return 0; /* timed out */
}
```

The `step` is capped at 10ms so that a cancel is noticed within 10ms even if
the overall timeout is a minute. That is the whole trick: **short waits in a
loop**, not one long wait.

The return values are a small contract worth testing in full:

- positive: bytes read
- `0`: the timeout elapsed with no data
- `-1`: cancelled or a real error

```c
static void test_times_out_when_no_data(void)
{
    /* ... empty pipe ... */
    CHECK_INT(read_with_context(fds[0], &ctx, buf, sizeof buf, 30), 0);
}

static void test_reports_cancellation(void)
{
    /* ... */
    read_context_cancel(&ctx);
    CHECK_INT(read_with_context(fds[0], &ctx, buf, sizeof buf, 1000), -1);
}
```

## Copying, and cancellation from another thread

The realistic use is a copy loop that pumps data until the source ends or the
operation is cancelled:

```c
int copy_with_context(int in_fd, int out_fd, const struct read_context *ctx,
                      int timeout_ms)
{
    int total = 0;
    char buffer[256];

    for (;;) {
        int n = read_with_context(in_fd, ctx, buffer, sizeof buffer, timeout_ms);

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
```

The test that matters is the one where nothing is ever written, and another
thread cancels:

```c
static void test_copy_is_interrupted_by_cancellation(void)
{
    /* ... */
    pthread_create(&thread, NULL, cancel_soon, &args);

    int n = copy_with_context(in[0], out[1], &ctx, -1); /* wait forever */
    CHECK_INT(n, -1);
    CHECK_TRUE(read_context_done(&ctx));
}
```

With a real `read`, this test would hang forever. With the context-aware
reader, it returns in about 20ms. Run it under ThreadSanitizer (`-DENABLE_TSAN=ON`)
and it is clean, because the only shared state is the atomic flag.

> This is the same lesson as the [time](time.md) chapter, in a different
> costume: the reason the test is easy is that we designed the I/O so it could
> be interrupted. A blocking `read` is untestable; a `poll` loop with a flag is
> trivial.

## Refactor

There is a subtlety in `copy_with_context`: a timeout returns the bytes copied
so far rather than an error. That is a choice — "stop for now, I can be called
again" — and it is the right one for a server that wants to do other work, but
a caller expecting "copy everything or fail" would be surprised. Document the
contract, or provide a variant. The interface is small enough that either is
easy.

If you take this further, you would integrate it with the event loop from the
[select](select.md) chapter, so one `poll` waits on many descriptors *and* the
cancel signal at once, instead of polling each reader in turn.

## Wrapping up

What we have covered:

- Making a blocking read cancellable with `poll` and an `atomic_bool`
- Short waits in a loop, so cancellation is noticed promptly
- A clear return contract: bytes, `0` for timeout, `-1` for cancel/error
- A copy loop, tested with cancellation from another thread
- Why designing for interruption is what makes the test possible

### Additional material

- [Context-aware `io.Reader` for Go](https://pace.dev/blog/2020/02/03/context-aware-ioreader-for-golang-by-mat-ryer) — the article this chapter is based on
- [`poll` man page](https://man7.org/linux/man-pages/man2/poll.2.html)
