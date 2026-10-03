# Mocking

**[You can find all the code for this chapter here](mocking/)**

We are going to write a countdown that prints `3`, `2`, `1`, `Go!` with a
one-second pause between each. The pause is the problem: if the test actually
sleeps for four seconds it will be slow, and slow tests stop being run.

The answer is to inject the sleeper, and then use a **spy** to check it was
asked to sleep the right number of times.

## Write the test first

```c
struct spy {
    int calls;
    int last_seconds;
};

static void spy_sleep(void *ctx, int seconds)
{
    struct spy *s = ctx;
    s->calls++;
    s->last_seconds = seconds;
}

static void test_countdown(void)
{
    struct buffer b;
    struct spy spy = {0};
    struct sleeper sleeper = {.sleep = spy_sleep, .ctx = &spy};

    buffer_init(&b);
    countdown(buffer_writer(&b), sleeper);

    CHECK_STR(b.data, "3\n2\n1\nGo!");
    CHECK_INT(spy.calls, 4);
    CHECK_INT(spy.last_seconds, 1);

    buffer_free(&b);
}
```

This test makes two kinds of assertion, and both matter:

- **Behaviour**: the output is `3\n2\n1\nGo!`
- **Interaction**: the sleeper was called four times, with one second each

The `spy` is a `struct sleeper` whose implementation records what happened
instead of sleeping. Because `struct sleeper` is a function pointer plus a
context, a spy is trivial: the context is the spy itself.

## Write enough code to make it pass

```c
void countdown(struct writer out, struct sleeper s)
{
    for (int i = 3; i > 0; i--) {
        char line[4];
        snprintf(line, sizeof line, "%d\n", i);
        writer_write(out, line);
        s.sleep(s.ctx, 1);
    }

    writer_write(out, "Go!");
    s.sleep(s.ctx, 1);
}
```

The real program wires in the real `sleep`:

```c
static void real_sleep(void *ctx, int seconds)
{
    (void)ctx;
    struct timespec ts = {.tv_sec = seconds};
    nanosleep(&ts, NULL);
}

int main(void)
{
    countdown(stdout_writer(), (struct sleeper){.sleep = real_sleep});
}
```

And the test wires in the spy. The function does not know or care which it got.

## Fakes, stubs and spies

These words get used loosely, so here is how I use them:

- A **stub** returns canned answers. It does not record anything.
- A **spy** records how it was called so the test can assert on it.
- A **fake** is a working implementation suited to tests — an in-memory map
  instead of a database, say.

Our `spy` is a spy, and the `buffer` from the last chapter is a fake (it really
does store the bytes). Notice that neither of them is a "mock" in the strict
sense of a library that records expectations. We just wrote two small structs.

That is the punchline of this chapter: in C, you rarely need a mocking
framework. An interface is a struct of function pointers, so a test double is a
struct of test functions. There is nothing to install.

## Refactor: don't assert on things you don't care about

`spy.calls == 4` is a meaningful assertion: the countdown should pause between
each number *and* before `Go!`. `spy.last_seconds == 1` is a stand-in for
"every call asked for one second". In a richer spy you would record all the
arguments in an array and assert on the whole sequence.

The trap to avoid is over-specifying. A test that asserts *exactly* how many
times an internal helper was called is brittle: it will fail the next time you
refactor, even though the behaviour is correct. Assert on interactions that are
part of the contract. Here, "we pause for each step" is part of the contract;
"we call `writer_write` exactly four times" is not.

## Wrapping up

What we have covered:

- Making time-dependent code testable by injecting the clock/sleeper
- A spy as a small struct, with no mocking framework
- Behaviour assertions versus interaction assertions
- The vocabulary of stubs, fakes and spies
- Why over-specifying interactions makes tests brittle

Go's chapter on this uses `time.Sleep` and a `Sleeper` interface. Here it is a
function pointer and a context. The idea is identical, and so is the advice:
inject what hurts, and assert on what you actually care about.

### Additional material

- [Mocks aren't stubs](https://martinfowler.com/articles/mocksArentStubs.html) — the article that named the vocabulary
