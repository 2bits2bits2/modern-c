# Scaling acceptance tests

**[You can find all the code for this chapter here](scaling-acceptance-tests/)**

Acceptance tests are valuable and expensive. Each one starts a process, waits
for it to come up, talks to it, and tears it down. Do that naively and your
suite becomes slow and flaky — and flaky tests are worse than no tests,
because people stop believing them. This chapter is a handful of techniques
that keep acceptance tests fast and trustworthy.

## Technique 1: poll for readiness, never sleep

Here is the single most common source of flakiness. A test starts the server,
"sensibly" waits a moment, and connects:

```c
struct timespec ts = {.tv_sec = 0, .tv_nsec = 50 * 1000 * 1000};
nanosleep(&ts, NULL);                    /* "give it a moment" */
int fd = accept_connect_unix(socket_path, 0);  /* one attempt */
CHECK_TRUE(fd >= 0);
```

It works on your laptop. It fails under load, or on CI, or when the server got
slightly slower. Here is that exact test failing, with a server that takes
300ms to start:

```text
    naive.c:14: test_naive_fixed_sleep: expected true: fd >= 0
--- FAIL test_naive_fixed_sleep

1 test(s), 1 check(s), 1 failed
FAIL
```

The failure is not about the server. It is about the test guessing how long
the server takes. The fix is to **poll until it is ready**, with a timeout that
is generous but bounded. That is what `accept_connect_unix` does:

```c
int fd = accept_connect_unix(socket_path, 2000);
```

It retries every few milliseconds until 2000ms have passed, and returns as
soon as it can connect. On a fast machine that is a few milliseconds; on a
slow one it waits exactly as long as it must. The test is now both faster and
more reliable, which is the dream.

Our acceptance suite has a test for this (a server that deliberately delays
its startup by 300ms):

```c
static void test_slow_startup_is_waited_for(void)
{
    char socket_path[128];
    char delay[] = "300";
    /* ... spawn with delay, then: */
    int fd = accept_connect_unix(socket_path, 2000);
    CHECK_TRUE(fd >= 0);
    /* ... */
}
```

## Technique 2: give every test its own resources

Two acceptance tests running at once must not fight over the same socket path.
We derive the path from the process id, and a counter so repeated starts within
one process are also unique:

```c
void accept_socket_path(char *buf, size_t cap, const char *name)
{
    static int counter;

    counter++;
    snprintf(buf, cap, "/tmp/mctest-%s-%d-%d.sock", name, (int)getpid(), counter);
}
```

This is the acceptance-test version of "no global mutable state". Shared, fixed
paths are a classic way to get a test that passes alone and fails in the suite.

## Technique 3: always clean up

Every process you start must be stopped, every socket file unlinked, even when
a check fails. In our tests this is partly handled because a failed `CHECK_*`
does not `abort` — the harness keeps going — so the teardown lines still run.
But if a test *can* return early, make the teardown unconditional, or reach for
the **fixture**.

## A fixture

A fixture packages setup and teardown so every test that needs a server gets
one without repeating five lines:

```c
struct server_fixture {
    pid_t pid;
    char socket_path[128];
};

bool fixture_start(struct server_fixture *f, const char *server_path);
int fixture_connect(struct server_fixture *f);
bool fixture_stop(struct server_fixture *f);
```

`fixture_start` spawns the server and does the readiness probe; `fixture_stop`
terminates it and unlinks the path. A test becomes:

```c
static void test_round_trip_with_a_fixture(void)
{
    struct server_fixture f;

    CHECK_TRUE(fixture_start(&f, server_path));

    int fd = fixture_connect(&f);
    CHECK_TRUE(fd >= 0);

    if (fd >= 0) {
        char reply[64];

        accept_roundtrip(fd, "ping", reply, sizeof reply);
        CHECK_STR(reply, "ping");
        close(fd);
    }

    CHECK_TRUE(fixture_stop(&f));
}
```

`fixture_stop` checks that the process exited cleanly, so the lifecycle
assertion lives in one place instead of in every test.

> A fixture is just a helper. In Go you would mark it with `t.Helper()` so
> failures point at the caller. In C there is no such mechanism, which is one
> more reason to keep fixtures small and their assertions few.

## Prove isolation with a stress loop

The best way to trust that your fixtures and unique paths work is to run a lot
of them. This test starts 25 fresh servers in a row:

```c
static void test_many_fresh_servers(void)
{
    for (int i = 0; i < 25; i++) {
        struct server_fixture f;

        CHECK_TRUE(fixture_start(&f, server_path));

        int fd = fixture_connect(&f);
        CHECK_TRUE(fd >= 0);

        if (fd >= 0) {
            char reply[64];

            accept_roundtrip(fd, "hi", reply, sizeof reply);
            CHECK_STR(reply, "hi");
            close(fd);
        }

        CHECK_TRUE(fixture_stop(&f));
    }
}
```

If any resource leaked — a socket file not unlinked, a child not reaped — a
loop of 25 will usually expose it, and the sanitizers will confirm it.

## Where acceptance tests stop

It is tempting to write *everything* as an acceptance test, because it feels
so real. Resist. They are slower and their failures are less precise. Use them
for:

- Wiring: does `main` assemble the pieces correctly?
- Protocols and formats: the actual bytes and the actual exit codes
- Lifecycle: startup, shutdown, signals, cleanup

Keep the bulk of your logic in functions with unit tests. In this chapter the
greeting is unit-tested; only the socket/process behaviour is acceptance-tested.
That ratio — many unit tests, a few acceptance tests — is the one that scales.

## Wrapping up

What we have covered:

- Poll for readiness with a timeout; never `sleep` and hope
- Unique resources per test (socket paths from pid + counter)
- Unconditional cleanup, and the fixture that packages it
- A stress loop as a cheap way to detect leaked resources
- What belongs in an acceptance test, and what belongs in a unit test

Go addresses the same problems with `t.Helper`, `testcontainers` and build
tags. The shapes differ; the instincts are identical.

### Additional material

- [Test flakiness: causes and cures](https://testing.googleblog.com/2016/05/flaky-tests-at-google-and-how-we.html)
