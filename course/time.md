# Time

**[You can find all the code for this chapter here](time/)**

Our product owner runs poker tournaments, and the blinds — the forced bets —
go up every few minutes. The server needs to know the current blind level, and
it needs to schedule an alert at each change. The catch is the one that has
haunted testing forever: anything that depends on the clock is slow and flaky
to test.

The fix is the same trick we used for the countdown in the
[mocking chapter](mocking.md), now made first-class: **inject the clock**.

## Just enough: a clock is an interface

```c
struct clock {
    long (*now_ms)(void *ctx);
    void *ctx;
};

long clock_now_ms(const struct clock *clock);
struct clock clock_system(void);
```

Production code calls `clock_system()` and gets the real monotonic clock.
Tests call `fake_clock_as_clock()` on a value they control:

```c
struct fake_clock fake = {.now_ms = 10 * 1000};
struct clock clock = fake_clock_as_clock(&fake);
...
fake_clock_advance(&fake, 30 * 1000);
```

`CLOCK_MONOTONIC` is the right production clock: unlike the wall clock, it
only ever moves forward, so it cannot be thrown off when the system adjusts the
time.

## Write the test first: the blind level

```c
static void test_blind_levels(void)
{
    struct fake_clock fake = {.now_ms = 10 * 1000};
    struct clock clock = fake_clock_as_clock(&fake);

    /* The game started at 10s; each level lasts one minute. */
    CHECK_INT(blind_level(&clock, 10 * 1000, 1), 0);

    fake_clock_advance(&fake, 30 * 1000); /* 40s in */
    CHECK_INT(blind_level(&clock, 10 * 1000, 1), 0);

    fake_clock_advance(&fake, 30 * 1000); /* 60s in: level 1 */
    CHECK_INT(blind_level(&clock, 10 * 1000, 1), 1);
}
```

The test covers an hour of play in microseconds, and it will never fail because
the machine was busy.

## Write enough code to make it pass

```c
int blind_level(const struct clock *clock, long start_ms, int level_minutes)
{
    long elapsed = clock_now_ms(clock) - start_ms;
    long level_ms = (long)level_minutes * 60L * 1000L;

    if (elapsed < 0) {
        return 0;
    }

    return (int)(elapsed / level_ms);
}
```

Integer division gives the level directly. The `elapsed < 0` guard handles a
clock that has been set before the start time; without it, a negative elapsed
would produce a large positive level, or a negative one cast to `int`. Guard
the boundary, then do the simple arithmetic.

## Scheduling: don't sleep, run due work

A server cannot sit in `nanosleep` waiting for the next blind level; it has to
serve requests meanwhile. So the scheduler does not wait at all. It holds
callbacks with due times, and someone calls `scheduler_run` when convenient —
after `poll` times out, or on every loop iteration.

```c
bool scheduler_at(struct scheduler *scheduler, long offset_ms,
                  void (*fn)(void *ctx), void *ctx);

size_t scheduler_run(struct scheduler *scheduler);
```

`run` fires every callback whose due time has arrived, exactly once, and
returns how many fired.

## Write the test first

```c
static void test_callbacks_fire_in_time_order(void)
{
    struct fake_clock fake = {.now_ms = 0};
    struct clock clock = fake_clock_as_clock(&fake);
    struct scheduler scheduler;
    struct counter counter = {0};

    scheduler_init(&scheduler, &clock, 0);
    scheduler_at(&scheduler, 10, on_fire, &counter);
    scheduler_at(&scheduler, 20, on_fire, &counter);

    CHECK_INT(scheduler_run(&scheduler), 0);   /* nothing due yet */

    fake_clock_advance(&fake, 10);
    CHECK_INT(scheduler_run(&scheduler), 1);   /* first fires */
    CHECK_INT(counter.fired, 1);

    CHECK_INT(scheduler_run(&scheduler), 0);   /* not twice */

    fake_clock_advance(&fake, 10);
    CHECK_INT(scheduler_run(&scheduler), 1);   /* second fires */
    CHECK_INT(counter.fired, 2);
}
```

Three properties are pinned down here: a callback does not fire early, fires
exactly once, and the two callbacks fire in the right order. All three are
things a sleeping implementation could get subtly wrong.

## Write enough code to make it pass

```c
size_t scheduler_run(struct scheduler *scheduler)
{
    long now = clock_now_ms(scheduler->clock);
    size_t fired = 0;

    for (size_t i = 0; i < scheduler->len; i++) {
        struct scheduled *item = &scheduler->items[i];

        if (!item->fired && now >= item->due_ms) {
            item->fired = true;
            item->fn(item->ctx);
            fired++;
        }
    }

    return fired;
}
```

The `fired` flag is what makes it "exactly once". Without it, running the
scheduler twice at the same instant would fire everything twice.

## Refactor

There is an O(n) scan per `run`. For a handful of blind alerts that is
irrelevant. A real server with thousands of timers would keep the due times in
a heap. Notice the trade and choose deliberately — and notice that the
*interface* would not change, so a heap implementation could drop in behind
`scheduler_at`/`scheduler_run` without touching a single test that uses them.
That is the payoff of putting time behind an interface.

## Wrapping up

What we have covered:

- An injectable clock as a function pointer plus a context
- `CLOCK_MONOTONIC`, and why it beats the wall clock
- Testing an hour of logic in microseconds with a fake clock
- A scheduler that never sleeps, driven by explicit `run` calls
- Firing callbacks exactly once, and why that needs a flag
- That an interface lets you change the implementation later without changing
  the tests

### Additional material

- [`clock_gettime` man page](https://man7.org/linux/man-pages/man3/clock_gettime.3.html)
