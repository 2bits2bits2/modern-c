# Context

**[You can find all the code for this chapter here](context/)**

You have started some work that might run for a while — a download, a search,
a background loop. The user navigates away. How do you tell the work to stop?

Go's answer is `context.Context`: you pass one everywhere, and any of them can
be cancelled, which unblocks everything waiting on it. C has no such type, so
we build a small one. It turns out to be an atomic boolean plus a convention.

## Just enough cancellation

The whole idea is: some code checks a flag, over and over, and stops when it is
set. The flag must be safe to set from another thread, so it is an
`atomic_bool` from [`<stdatomic.h>`](sync.md).

```c
struct context {
    atomic_bool cancelled;
};

void context_init(struct context *c);
void context_cancel(struct context *c);
bool context_done(const struct context *c);
```

`context_done` is the check; `context_cancel` is the signal. That is all a
context is.

## Write the test first

```c
static void test_a_new_context_is_not_done(void)
{
    struct context c;

    context_init(&c);

    CHECK_TRUE(!context_done(&c));
}

static void test_cancel_marks_it_done(void)
{
    struct context c;

    context_init(&c);
    context_cancel(&c);

    CHECK_TRUE(context_done(&c));
}
```

Then the interesting one: a worker that loops until cancelled.

```c
struct worker {
    const struct context *ctx;
    long iterations;
};

static void *run_worker(void *arg)
{
    struct worker *w = arg;

    while (!context_done(w->ctx)) {
        w->iterations++;
    }

    return NULL;
}
```

## Write enough code to make it pass

```c
void context_init(struct context *c)
{
    atomic_init(&c->cancelled, false);
}

void context_cancel(struct context *c)
{
    atomic_store(&c->cancelled, true);
}

bool context_done(const struct context *c)
{
    return atomic_load(&c->cancelled);
}
```

The test starts the worker, lets it spin for two milliseconds, cancels, and
joins:

```c
context_init(&c);
pthread_create(&thread, NULL, run_worker, &w);

nanosleep(&pause, NULL);
context_cancel(&c);
pthread_join(thread, NULL);

CHECK_TRUE(w.iterations > 0);
CHECK_TRUE(context_done(&c));
```

`w.iterations` is read after `pthread_join`, which synchronises the two
threads, so the read is safe. Notice the worker burns CPU while it waits; in
real code you would block on `poll` or a condition variable and have the cancel
wake it. We are keeping the mechanism bare so you can see it.

## Refactor: propagating and deadlines

A single boolean is the seed. Real contexts add two things:

**Propagation.** When a parent is cancelled, its children should be too. The
usual C shape is to give each child a pointer to its parent, and have
`context_done` walk up the chain:

```c
bool context_done(const struct context *c)
{
    for (; c != NULL; c = c->parent) {
        if (atomic_load(&c->cancelled)) {
            return true;
        }
    }
    return false;
}
```

**Deadlines.** Store an absolute deadline (from `clock_gettime`), and have
`context_done` also return true once we are past it. A cancel and a timeout are
then the same signal to the worker.

Both are small extensions of the same idea. The hard part is not the context —
it is threading the `struct context *` through every function that might block,
which is a design decision more than a technical one.

## Wrapping up

What we have covered:

- A context as a shared, atomically-cancellable flag
- Checking cancellation in a long-running loop
- Reading shared state after `pthread_join`
- Extending the idea with parent propagation and deadlines
- Why the difficulty is passing the context around, not the flag itself

Go bakes `context.Context` into its standard library and its conventions. C
gives you the primitives to build the same thing, which means you can also see
exactly how much it costs: one atomic load per check.

### Additional material

- [Go blog: context](https://go.dev/blog/context) — worth reading even in C; it is about design, not syntax
