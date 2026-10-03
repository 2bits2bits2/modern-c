# Introduction to acceptance tests

**[You can find all the code for this chapter here](acceptance-tests/)**

We have a program to write: a little greeting server. It listens on a socket,
answers a request with `Hello, <name>!`, and — this is the requirement that
makes the chapter interesting — it must **shut down gracefully** when asked.

Up to now our tests have called functions. But the thing we actually ship is a
*program*: it binds a socket, handles connections, and responds to signals.
Unit tests cannot tell us whether the program's `main` wires all that together
correctly. That is what an **acceptance test** is for: it treats the program as
a black box, starts it for real, talks to it over its real interface, and
checks the behaviour a user would see.

> A unit test asks "is this function correct?" An acceptance test asks "does
> the whole thing work?" They are different questions, and you want both.

## Write the unit test first

The greeting itself is ordinary domain logic, so test it directly:

```c
static void test_greeting(void)
{
    char buf[64];

    greeting("Chris", buf, sizeof buf);
    CHECK_STR(buf, "Hello, Chris!");

    greeting("", buf, sizeof buf);
    CHECK_STR(buf, "Hello, !");
}
```

The implementation builds the string piece by piece so there is no fixed-size
`snprintf` to truncate:

```c
void greeting(const char *name, char *buf, size_t n)
{
    const char *parts[3] = {"Hello, ", name, "!"};
    size_t pos = 0;

    if (n == 0) {
        return;
    }

    for (int p = 0; p < 3; p++) {
        for (const char *s = parts[p]; *s != '\0' && pos + 1 < n; s++) {
            buf[pos] = *s;
            pos++;
        }
    }

    buf[pos] = '\0';
}
```

Then a program around it: bind an `AF_UNIX` socket at a path given on the
command line, accept connections, read a name, write the greeting.

## The acceptance test

The test needs to launch that program. The book provides a small helper,
[`mcaccept`](test/mcaccept.h), alongside the `mctest` harness:

- `accept_socket_path` builds a unique socket path from the process id
- `accept_spawn` forks and execs the server
- `accept_connect_unix` connects, retrying until a timeout
- `accept_roundtrip` writes a request and reads the reply
- `accept_terminate` sends `SIGTERM` and waits for the child to exit

With those, the test reads almost like a description of the requirement:

```c
static void test_greets_over_a_unix_socket(void)
{
    char socket_path[128];
    char *argv[3];

    accept_socket_path(socket_path, sizeof socket_path, "accept-v1");
    unlink(socket_path);

    argv[0] = (char *)server_path;
    argv[1] = socket_path;
    argv[2] = NULL;

    pid_t pid = accept_spawn(server_path, argv);
    CHECK_TRUE(pid > 0);

    int fd = accept_connect_unix(socket_path, 2000);
    CHECK_TRUE(fd >= 0);

    if (fd >= 0) {
        char reply[128];

        accept_roundtrip(fd, "Chris", reply, sizeof reply);
        CHECK_STR(reply, "Hello, Chris!");
        close(fd);
    }

    int status = 0;
    CHECK_TRUE(accept_terminate(pid, 2000, &status));
    CHECK_TRUE(WIFSIGNALED(status));

    unlink(socket_path);
}
```

The server program's path arrives as a command-line argument, because CMake
passes `$<TARGET_FILE:...>` to the test. That is the only plumbing the test
needs to know about the build.

Run it and you get:

```text
--- PASS test_greets_over_a_unix_socket

1 test(s), 6 check(s), 0 failed
PASS
```

Nothing in the test calls `greeting`. It never includes `server.c`. It knows
only the wire protocol and the socket path. That is the point: if you rewrote
the server in a different style — or a different language — this test would
not change.

## More requirements: shut down gracefully

Right now the server has no signal handling, so `SIGTERM` kills it outright.
Look at what the shell sees:

```text
$ ./acceptance_v1_server /tmp/sigstart.sock &
$ kill -TERM $!
$ wait $!
exit status: 143
```

`143` is `128 + 15`: the shell reporting that the process was terminated by
signal 15 (`SIGTERM`). The process never got to clean up. That is not the
behaviour we want. A well-behaved server finishes what it is doing, releases
the socket, removes the socket file, and exits with a normal `0`.

### Write the test first

Change the test to demand a clean exit:

```c
    CHECK_TRUE(accept_terminate(pid, 2000, &status));
    CHECK_TRUE(WIFEXITED(status));
    CHECK_INT(WEXITSTATUS(status), 0);
```

Run it against the old server and it fails — `WIFEXITED` is false because the
process was signalled. Exactly the failure we expect.

### Write enough code to make it pass

Install a handler that flips a flag, and let `accept` be interrupted by it:

```c
static volatile sig_atomic_t running = 1;

static void on_signal(int signal_number)
{
    (void)signal_number;
    running = 0;
}

static int install_signal_handlers(void)
{
    struct sigaction action;

    memset(&action, 0, sizeof action);
    action.sa_handler = on_signal;

    if (sigaction(SIGTERM, &action, NULL) != 0 ||
        sigaction(SIGINT, &action, NULL) != 0) {
        return -1;
    }
    return 0;
}
```

and the loop becomes:

```c
while (running) {
    int conn = accept(listener, NULL, NULL);

    if (conn < 0) {
        if (errno == EINTR) {
            continue;
        }
        break;
    }

    handle(conn);
    close(conn);
}

close(listener);
unlink(argv[1]);
return 0;
```

Two details carry the whole design:

- `signal` handlers may only safely touch `volatile sig_atomic_t` and call
  async-signal-safe functions. Setting a flag is the canonical safe thing to
  do; calling `printf` or `free` inside a handler is not.
- We do **not** set `SA_RESTART`, so the blocking `accept` returns `-1` with
  `errno == EINTR` when the signal arrives. That is how the loop gets a chance
  to notice `running == 0`. If we had set `SA_RESTART`, `accept` would resume
  silently and the shutdown would be delayed until the next connection.

Now the acceptance test passes, and the shell agrees:

```text
$ ./acceptance_v2_server /tmp/sigstart.sock &
$ kill -TERM $!
$ wait $!
exit status: 0
```

## Refactor

The test still has repeated connection bookkeeping. Notice how the `if (fd >= 0)`
guard keeps the test meaningful even when the connect fails: rather than
crashing, the remaining checks report the real problem. That defensive style is
worth keeping in acceptance tests, where a single failure can otherwise
cascade into a confusing mess.

The next chapter, [scaling acceptance tests](scaling-acceptance-tests.md),
takes the repetition and turns it into a fixture.

## Wrapping up

What we have covered:

- The difference between unit tests (functions) and acceptance tests (the
  whole program)
- Driving a real process: spawn, connect over a Unix socket, terminate
- `mcaccept`, the book's process/acceptance helper
- Graceful shutdown: `sigaction`, a `volatile sig_atomic_t` flag, and `EINTR`
- Reading a process's fate from `waitpid`: `WIFSIGNALED` versus `WIFEXITED`
- Why `SA_RESTART` is the wrong choice for a shutdown signal

This is the most realistic test we have written. It exercises the parts of C
that unit tests never reach: `main`, signals, sockets, and the process
lifecycle.

### Additional material

- [`signal` and `sigaction`](https://man7.org/linux/man-pages/man7/signal.7.html)
- [`waitpid` and the status macros](https://man7.org/linux/man-pages/man2/waitpid.2.html)
