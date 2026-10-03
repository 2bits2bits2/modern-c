# Revisiting time, with a virtual clock

**[You can find all the code for this chapter here](revisiting-time/)**

Go 1.25 added `testing/synctest`, a package that runs time-dependent code
against a fake clock so tests are instant and deterministic. C has nothing like
it — but the previous chapter showed we do not need a standard-library package
to get the same result. We just need to own the clock.

This chapter takes the injected clock and scheduler from the
[time chapter](time.md) and simulates an entire tournament.

## The idea: a virtual clock

Here is the whole trick, restated: if `time` is a dependency you pass in, then
"advance an hour" is a function call, not an hour. `fake_clock_advance` moves
the virtual clock forward; `scheduler_run` fires everything that became due.

```c
void tournament_advance(struct tournament *tournament, long ms)
{
    fake_clock_advance(&tournament->fake, ms);
    scheduler_run(&tournament->scheduler);
}
```

There is no `sleep`. There is no wall clock. The test decides that an hour has
passed, and it is true instantly.

## Write the test first

```c
static void test_a_whole_tournament(void)
{
    struct tournament tournament;

    tournament_start(&tournament, 3, 1); /* 3 levels, 1 minute each */

    tournament_advance(&tournament, 30 * 1000);
    CHECK_INT(tournament_alerts(&tournament), 0);

    tournament_advance(&tournament, 30 * 1000); /* 1 minute */
    CHECK_INT(tournament_alerts(&tournament), 1);

    tournament_advance(&tournament, 60 * 1000); /* 2 minutes */
    CHECK_INT(tournament_alerts(&tournament), 2);

    tournament_advance(&tournament, 60 * 1000); /* 3 minutes */
    CHECK_INT(tournament_alerts(&tournament), 3);

    tournament_advance(&tournament, 60 * 60 * 1000); /* an hour later */
    CHECK_INT(tournament_alerts(&tournament), 3);
}
```

This is the payoff. The test reads like a description of how a tournament
behaves, and it runs in microseconds. The same test against a real clock would
take three minutes and occasionally fail on a loaded CI machine.

## Write enough code to make it pass

The tournament wires the fake clock to the scheduler and schedules one alert
per level:

```c
void tournament_start(struct tournament *tournament, int levels,
                      int level_minutes)
{
    tournament->fake.now_ms = 0;
    tournament->clock = fake_clock_as_clock(&tournament->fake);
    tournament->alerts = 0;

    scheduler_init(&tournament->scheduler, &tournament->clock, 0);
    for (int i = 1; i <= levels; i++) {
        scheduler_at(&tournament->scheduler,
                     (long)i * level_minutes * 60L * 1000L, on_alert,
                     tournament);
    }
}
```

There is no timing code here at all — no sleeps, no threads. The scheduling is
pure data until `scheduler_run` calls the callbacks.

## Why this is better than sleeping

The alternative, testing against the real clock, has three problems that never
go away:

- **It is slow.** Every real second is a second your suite spends not finding
  bugs.
- **It is flaky.** A test that sleeps 100ms works until the CI machine is busy,
  and then it fails, and then you add a longer sleep, and then it is slower
  *and* still flaky.
- **It cannot test the edges cheaply.** You want to check what happens at
  exactly the boundary — one millisecond before the level changes. With a
  virtual clock that is trivial; with a real clock it is a coin toss.

Go's `synctest` is a runtime feature that cooperates with goroutines and timers
to give the same guarantee for *existing* code. C's version is a design
decision you make up front: put time behind an interface. The cost is that you
must design for it; the benefit is that it works in any language, in any C
project, without a special test runtime.

> If you take one habit from this section, take this one: never call a
> time-reading function directly from your logic. Wrap it. The day you need to
> test a deadline, a timeout, or a schedule, you will already have done the
> hard part.

## Refactor

`struct tournament` owns both the fake clock and the scheduler, which is
convenient for a test but slightly muddles "the system under test" with "the
test harness". A production tournament would receive a `struct clock` from its
caller and not know whether it is real. The version here is the test's-eye
view; feel free to split it, and watch how few lines it takes now that the
clock is already a parameter.

## Wrapping up

What we have covered:

- Simulating an entire timed event on a virtual clock
- Why deterministic time beats sleeping, in three concrete ways
- Designing time behind an interface as C's answer to `testing/synctest`
- Testing exact boundaries cheaply
- The one habit: never read the clock directly from your logic

### Additional material

- [Go 1.25 `testing/synctest`](https://pkg.go.dev/testing/synctest) — worth reading for how another ecosystem solved the same problem
