# Concurrency

**[You can find all the code for this chapter here](concurrency/)**

We need to check whether a list of websites are up. Checking them one at a
time is correct but slow; the network is the bottleneck and we are using none
of it. This is the chapter where C stops being a single-threaded language.

## Just enough pthreads

Go's concurrency story is goroutines and channels. C's is **threads**, and on
POSIX systems the API is [pthreads](https://en.wikipedia.org/wiki/Pthreads).
The core of it is four functions:

```c
pthread_t t;
pthread_create(&t, NULL, function, argument);  /* start a thread   */
pthread_join(t, NULL);                         /* wait for it       */
pthread_mutex_lock(&m);                        /* enter a critical section */
pthread_mutex_unlock(&m);                      /* leave it          */
```

A thread runs a function of the signature `void *f(void *)`. Whatever you pass
as the last argument to `pthread_create` arrives as that `void *`. This is the
same type-erasure trick as the `struct shape` from earlier.

Build with `-pthread` (which our CMake does via `Threads::Threads`), and make
sure each thread writes to memory that no other thread touches, or takes a lock
first.

## Write the test first

```c
static bool fake_check(const char *url)
{
    return url[0] == 'h';
}

static void test_concurrent(void)
{
    struct website sites[] = {
        {.url = "https://a", .ok = false},
        {.url = "https://b", .ok = false},
        {.url = "ftp://c", .ok = false},
    };

    check_all_concurrent(sites, 3, fake_check);

    CHECK_TRUE(sites[0].ok);
    CHECK_TRUE(sites[1].ok);
    CHECK_TRUE(!sites[2].ok);
}
```

The `check_fn` is injected, exactly like the sleeper in the last chapter, so
the test does not need a network. Because each thread only writes
`sites[i].ok`, and no two threads share an `i`, this design needs no lock at
all. That is deliberate — the best way to avoid a data race is to not share
mutable state.

## Write enough code to make it pass

```c
struct job {
    struct website *site;
    check_fn check;
};

static void *check_one(void *arg)
{
    struct job *j = arg;
    j->site->ok = j->check(j->site->url);
    return NULL;
}

void check_all_concurrent(struct website *sites, size_t n, check_fn check)
{
    pthread_t *threads = malloc(n * sizeof *threads);
    struct job *jobs = malloc(n * sizeof *jobs);

    if (threads == NULL || jobs == NULL) {
        free(threads);
        free(jobs);
        check_all_sequential(sites, n, check);
        return;
    }

    for (size_t i = 0; i < n; i++) {
        jobs[i] = (struct job){.site = &sites[i], .check = check};
        pthread_create(&threads[i], NULL, check_one, &jobs[i]);
    }

    for (size_t i = 0; i < n; i++) {
        pthread_join(threads[i], NULL);
    }

    free(threads);
    free(jobs);
}
```

Two details worth noticing:

- We pass `&jobs[i]` to each thread. If we accidentally passed `&job` for a
  single shared job, the threads would trample each other. Each thread gets its
  own job.
- On allocation failure we fall back to the sequential version rather than
  crashing. The caller gets correct answers either way.

## Does it actually go faster?

This is the moment the [iteration chapter's benchmark](iteration.md) pays off.
We sleep 50ms in each check and compare:

```text
sequential: 0.401 s
concurrent: 0.051 s
```

Eight sites, eight threads, roughly the eight-fold speed-up you would hope for.
With real network calls the ratio depends on how slow the remote end is, but
the shape is the same. Measure it yourself on your own machine:

```sh
cmake --build build --target concurrency_v2_bench
./build/concurrency/concurrency_v2_bench
```

## When threads go wrong: data races

Sharing mutable state without synchronisation is the bug that concurrency
introduces. Here is a deliberately broken program: two threads increment a
plain `long` with no lock.

```c
static long total;

static void *add_many(void *arg)
{
    (void)arg;
    for (int i = 0; i < 100000; i++) {
        total++;
    }
    return NULL;
}
```

Compile it with ThreadSanitizer (our build has a `-DENABLE_TSAN=ON` option) and
run it, and it tells you exactly what is wrong:

```text
WARNING: ThreadSanitizer: data race (pid=383049)
  Read of size 8 at 0x555555558018 by thread T2:
    #0 add_many /tmp/opencode/race.c:9

  Previous write of size 8 at 0x555555558018 by thread T1:
    #0 add_many /tmp/opencode/race.c:9

  Location is global 'total' of size 8

SUMMARY: ThreadSanitizer: data race /tmp/opencode/race.c:9 in add_many
```

`total++` is three operations — read, add, write — and two threads can interleave
them so that one increment is lost. Notice that on this particular run the
program happened to print the correct answer anyway. **A race is a race even
when it does not bite you today.** That is exactly why you run the sanitizer
rather than waiting for the bug report.

A note for containers and some kernels: ThreadSanitizer can fail to start with
`FATAL: ThreadSanitizer: unexpected memory mapping`. That is an ASLR issue, not
your code. Prefix the command with `setarch $(uname -m) -R` to disable
randomisation for that run.

Move on to the [sync](sync.md) chapter to see the three ways to fix this.

## Wrapping up

What we have covered:

- `pthread_create` and `pthread_join`, and the `void *` thread function
- Passing each thread its own data to avoid sharing
- A benchmark that shows a real, measured speed-up
- What a data race is, and how ThreadSanitizer reports one
- The `setarch -R` workaround for the sanitizer in containers

One thread per task is simple and fine for eight websites. It does not scale to
eight thousand; for that you want a worker pool. But the primitives are the
same, and you now know what they cost.

### Additional material

- [ThreadSanitizer](https://clang.llvm.org/docs/ThreadSanitizer.html)
- [`pthread_create` man page](https://man7.org/linux/man-pages/man3/pthread_create.3.html)
