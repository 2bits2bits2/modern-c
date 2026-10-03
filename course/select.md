# Select

**[You can find all the code for this chapter here](select/)**

We have two inputs and we want to know which one to read from next, without
blocking forever if neither is ready. Go has `select`; C has `poll` (and the
older `select`, which `poll` replaced for most purposes). This chapter is about
waiting on several things at once, with a timeout.

## Just enough `poll`

`poll` takes an array of file descriptors and blocks until one of them is
ready or a timeout expires:

```c
struct pollfd fds[2] = {
    {.fd = fd_a, .events = POLLIN, .revents = 0},
    {.fd = fd_b, .events = POLLIN, .revents = 0},
};

int ready = poll(fds, 2, timeout_ms);
```

It returns the number of descriptors that are ready, `0` if it timed out, or
`-1` on error. It then sets `revents` on each descriptor to say what happened.
`POLLIN` means there is data to read.

A timeout of `-1` means "wait forever"; a timeout of `0` means "return
immediately" (a poll, not a wait).

## Write the test first

`poll` works on file descriptors, and the easiest pair to test with is a pipe.
`pipe(fds)` fills `fds[0]` with the read end and `fds[1]` with the write end.

```c
static void test_timeout(void)
{
    int a[2];
    int b[2];

    CHECK_INT(pipe(a), 0);
    CHECK_INT(pipe(b), 0);

    CHECK_INT(wait_ready(a[0], b[0], 20), READY_NONE);

    close(a[0]);
    close(a[1]);
    close(b[0]);
    close(b[1]);
}
```

And a test where the first pipe *is* ready:

```c
static void test_first_is_ready(void)
{
    int a[2];
    int b[2];

    pipe(a);
    pipe(b);
    CHECK_INT(write(a[1], "x", 1), 1);

    CHECK_INT(wait_ready(a[0], b[0], 1000), READY_FIRST);

    close(a[0]);
    close(a[1]);
    close(b[0]);
    close(b[1]);
}
```

## Write enough code to make it pass

```c
enum ready wait_ready(int fd_a, int fd_b, int timeout_ms)
{
    struct pollfd fds[2] = {
        {.fd = fd_a, .events = POLLIN, .revents = 0},
        {.fd = fd_b, .events = POLLIN, .revents = 0},
    };

    int ready = poll(fds, 2, timeout_ms);
    if (ready < 0) {
        return READY_ERROR;
    }
    if (ready == 0) {
        return READY_NONE;
    }

    if (fds[0].revents & POLLIN) {
        return READY_FIRST;
    }
    if (fds[1].revents & POLLIN) {
        return READY_SECOND;
    }

    return READY_ERROR;
}
```

The `enum ready` makes the result readable. Like the wallet errors, the values
are prefixed and `READY_NONE` is zero.

## Refactor

There is a fairness subtlety worth knowing about. If both descriptors are ready
at the same time, this code always reports the first. A real event loop usually
services both, so it would iterate over `fds`, service every descriptor with
`POLLIN` set, and only then call `poll` again. Our version answers a narrower
question — "which one should I look at first?" — which is all this chapter
needs.

The other thing to handle in harder code is `POLLHUP` and `POLLERR`. We fold
those into `READY_ERROR`, but a chat server would need to close the connection.

## Wrapping up

What we have covered:

- Waiting on multiple file descriptors with `poll`
- Timeouts: `-1` forever, `0` immediate, positive in milliseconds
- `revents` as a bitmask, tested with `&`
- Why an `enum` makes the result self-documenting
- Pipes as a convenient way to test anything descriptor-shaped

This is the machinery under every event loop: web servers, terminals, GUI
toolkits. `select`/`poll` is the C answer to Go's `select`, and it deals in
file descriptors rather than channels.

### Additional material

- [`poll` man page](https://man7.org/linux/man-pages/man2/poll.2.html)
- [The C10K problem](http://www.kegel.com/c10k.html) — the classic essay on handling many connections
