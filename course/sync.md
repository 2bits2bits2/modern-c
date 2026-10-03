# Sync

**[You can find all the code for this chapter here](sync/)**

The [concurrency chapter](concurrency.md) ended with a data race: two threads
incrementing a shared counter. This chapter is the three ways to fix it — a
mutex, a condition variable, and atomics — and when to reach for each.

## A mutex-protected counter

The simplest fix is mutual exclusion: only one thread may be inside the
increment at a time.

```c
struct counter {
    pthread_mutex_t mu;
    long value;
};

void counter_inc(struct counter *c)
{
    pthread_mutex_lock(&c->mu);
    c->value++;
    pthread_mutex_unlock(&c->mu);
}
```

### Write the test first

Eight threads, one hundred thousand increments each, then check the total:

```c
static void test_concurrent_increments(void)
{
    struct counter c;
    pthread_t threads[NUM_THREADS];
    struct worker_args args[NUM_THREADS];

    counter_init(&c);

    for (int i = 0; i < NUM_THREADS; i++) {
        args[i] = (struct worker_args){.counter = &c, .times = INCREMENTS};
        pthread_create(&threads[i], NULL, worker, &args[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    CHECK_INT(counter_value(&c), NUM_THREADS * INCREMENTS);

    counter_destroy(&c);
}
```

Run this one repeatedly, and under `-DENABLE_TSAN=ON`. Without the lock it is
exactly the racy program from the last chapter, and the number will sometimes
be wrong. With the lock it is always `800000`.

A word on getting it right: every path out of the critical section must unlock.
If `counter_inc` could `return` early or `longjmp`, you would need a cleanup
handler or a `goto` to a single unlock point. Keep critical sections small and
without early exits, and you will rarely need those tools.

`counter_value` takes the lock too, even though it only reads. That is because
a `long` read can still race with a write on some architectures, and because
the lock also orders memory: without it, the reading thread might not see the
latest values at all.

## A WaitGroup

Go's `sync.WaitGroup` lets you wait for a group of tasks without tracking
threads yourself. It is a mutex plus a condition variable plus a count, and
it is short enough to build here.

### Write the test first

```c
static void test_waitgroup(void)
{
    struct waitgroup wg;
    pthread_t threads[NUM_WORKERS];
    struct worker_args args[NUM_WORKERS];
    int slots[NUM_WORKERS] = {0};

    wg_init(&wg);
    wg_add(&wg, NUM_WORKERS);

    for (int i = 0; i < NUM_WORKERS; i++) {
        args[i] = (struct worker_args){.wg = &wg, .slot = &slots[i], .value = i + 1};
        pthread_create(&threads[i], NULL, worker, &args[i]);
    }

    wg_wait(&wg);

    for (int i = 0; i < NUM_WORKERS; i++) {
        CHECK_INT(slots[i], i + 1);
    }

    for (int i = 0; i < NUM_WORKERS; i++) {
        pthread_join(threads[i], NULL);
    }

    wg_destroy(&wg);
}
```

Each worker writes its own slot, then calls `wg_done`. After `wg_wait` returns,
every slot must be filled.

### Write enough code to make it pass

```c
void wg_wait(struct waitgroup *wg)
{
    pthread_mutex_lock(&wg->mu);
    while (wg->count > 0) {
        pthread_cond_wait(&wg->cond, &wg->mu);
    }
    pthread_mutex_unlock(&wg->mu);
}
```

The two details that people get wrong:

- The `while`, not an `if`. A condition variable can wake up spuriously or
  because of another thread; you must re-check the condition every time.
- `pthread_cond_wait` takes the mutex as an argument because it atomically
  releases the lock while waiting and re-acquires it before returning. That
  closes the window where you check the count and then block, in which a
  `wg_done` could otherwise be missed.

`wg_done` decrements and, when the count hits zero, calls
`pthread_cond_broadcast` to wake every waiter.

## Atomics: the lock-free option

For a simple counter you do not need a mutex at all. C11 onwards gives you
`<stdatomic.h>`:

```c
struct counter {
    atomic_long value;
};

void counter_inc(struct counter *c)
{
    atomic_fetch_add(&c->value, 1);
}
```

Atomics are indivisible and, by default, sequentially consistent: the compiler
and CPU are not allowed to reorder them in ways that break your reasoning. For
a counter they are simpler and faster than a mutex.

But atomics only help with *single* operations. The moment you need to check
then change — "if the balance is enough, subtract" — one atomic is not enough,
and you are back to a mutex. A useful rule:

- One variable, one operation: atomics
- Several variables, or a check-then-act: a mutex
- Waiting for a condition: a condition variable

## Wrapping up

What we have covered:

- A mutex making a check-then-act sequence atomic
- A condition variable and why `pthread_cond_wait` needs the mutex
- `while`, not `if`, around a condition wait
- `pthread_cond_broadcast` versus `pthread_cond_signal`
- Atomics for single operations, and their limits
- Testing concurrent code: run it many times, and under ThreadSanitizer

Go's `sync` package gives you `Mutex`, `WaitGroup`, `RWMutex`, `Once`, and
`atomic`. In C you assemble them from pthreads and `<stdatomic.h>`. You now
own the whole stack — which is a blessing and a responsibility.

### Additional material

- [`pthread_mutex` man page](https://man7.org/linux/man-pages/man3/pthread_mutex_lock.3.html)
- [C atomics from the ground up](https://research.swtch.com/).
