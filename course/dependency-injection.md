# Dependency Injection

**[You can find all the code for this chapter here](dependency-injection/)**

We need a `greet` function that says hello. Trivial to write, and — if we are
careless — impossible to test, because its only observable effect is what
appears on the screen. This chapter is about the small design change that
makes it testable: **dependency injection**.

## Write the test first

We want to check the greeting without printing it. The trick is to make
`greet` write to something we control:

```c
#include <stdlib.h>

#include "greet.h"
#include "mctest.h"

static void test_greet(void)
{
    char *buf = NULL;
    size_t size = 0;
    FILE *out = open_memstream(&buf, &size);

    greet(out, "Chris");
    fclose(out);

    CHECK_STR(buf, "Hello, Chris!");

    free(buf);
}
```

`open_memstream` is a POSIX function that gives you a `FILE *` whose bytes go
into a growing memory buffer instead of a file. It is the C equivalent of Go's
`bytes.Buffer`, and it is exactly the kind of tool that makes "I/O is hard to
test" go away.

## Write enough code to make it pass

```c
void greet(FILE *out, const char *name)
{
    fprintf(out, "Hello, %s!", name);
}
```

`greet` no longer decides *where* the greeting goes; the caller does. In
production you pass `stdout`, in a test you pass a memory stream. That is
dependency injection: instead of creating its own dependency, the function is
handed one.

## Refactor: don't depend on `FILE` if you can depend on less

`FILE *` works, but it is a fat dependency. It drags in buffering, file
descriptors and a whole subsystem. The function only needs one ability:
"accept some bytes". Let us describe exactly that, and nothing more.

```c
struct writer {
    int (*write)(void *ctx, const char *s, size_t n);
    void *ctx;
};

int writer_write(struct writer w, const char *s);
```

A `writer` is a function pointer plus the state it needs. This is the same
vtable idea from the [structs chapter](structs-and-function-pointers.md), with
only one method. It is C's `io.Writer`.

### A buffer that implements it

```c
struct buffer {
    char *data;
    size_t len;
    size_t cap;
};

void buffer_init(struct buffer *b);
void buffer_free(struct buffer *b);
struct writer buffer_writer(struct buffer *b);
```

`buffer_write` grows the buffer with `realloc` and keeps it NUL-terminated, so
a test can just hand `b.data` to `CHECK_STR`.

### The new `greet`

```c
void greet(struct writer out, const char *name)
{
    writer_write(out, "Hello, ");
    writer_write(out, name);
    writer_write(out, "!");
}
```

Notice we did not build a temporary string. There is no buffer to size, no
`snprintf`, no chance of truncating a long name. We just write the three
pieces in order. Smaller and safer at once.

And the test becomes:

```c
struct buffer b;
buffer_init(&b);

greet(buffer_writer(&b), "Chris");

CHECK_STR(b.data, "Hello, Chris!");

buffer_free(&b);
```

## Why this is worth the ceremony

You could argue that `greet` was easy to test with `open_memstream` and we did
not need `struct writer` at all. For `greet`, that is fair. The value shows up
when the dependency is expensive or slow:

- A database: swap in an in-memory implementation for tests
- A network client: swap in a fake that returns canned responses
- A clock: swap in one you can control, so tests do not `sleep`
- A random source: swap in one that is deterministic

Every one of these is "a big thing" reduced to "a small interface the code
actually needs". The [mocking](mocking.md) chapter takes this further, and a
later chapter on working without mocks argues about where to stop.

## Wrapping up

What we have covered:

- Dependency injection: pass a dependency in rather than constructing it
- `open_memstream` and `bytes.Buffer`-style testing of output
- Designing an interface (`struct writer`) as the *smallest* set of abilities
  your code needs
- A function pointer plus a context pointer as a C interface
- Why gilting the dependency makes tests fast and deterministic

The Go version of this chapter says "don't over-engineer; inject what hurts".
The same advice holds here. Start concrete, and introduce an interface when a
test actually needs it.

### Additional material

- [`open_memstream` man page](https://man7.org/linux/man-pages/man3/open_memstream.3.html)
