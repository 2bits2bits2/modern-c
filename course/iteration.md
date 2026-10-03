# Iteration

**[You can find all the code for this chapter here](iteration/)**

Our next requirement: a `repeat` function that repeats a string a given
number of times. Along the way we will meet C's loop, and then learn how to
*measure* the code we write, because in C performance is something you can
actually see.

## Write the test first

```c
#include "repeat.h"
#include "mctest.h"

static void test_repeat(void)
{
    char buf[32];

    repeat("a", 3, buf, sizeof buf);

    CHECK_STR(buf, "aaa");
}
```

## Try to run the test

Build it and read the compiler:

```text
iteration/v1/repeat_test.c:3:31: error: implicit declaration of function ‘repeat’ [-Werror=implicit-function-declaration]
    3 | static void test_repeat(void)
      |                               ^~~~~~
cc1: all warnings being treated as errors
```

No declaration. We know what to do by now.

## Write the minimal amount of code for the test to run and check the failing test output

`repeat.h`:

```c
#include <stddef.h>

void repeat(const char *s, int times, char *buf, size_t n);
```

`repeat.c`, doing the least possible work:

```c
#include "repeat.h"

void repeat(const char *s, int times, char *buf, size_t n)
{
    (void)s;
    (void)times;
    if (n > 0) {
        buf[0] = '\0';
    }
}
```

And the failure:

```text
    iteration/v1/repeat_test.c:10: test_repeat: got "" want "aaa"
--- FAIL test_repeat
```

## Write enough code to make it pass

Here is the loop. C's `for` has three clauses, separated by semicolons:

```c
for (initialise; condition; update) {
    ...
}
```

We use two loops: one for the number of repetitions, one for the characters of
the string. And we never write past `n - 1`, because the buffer must keep room
for the terminating `'\0'`.

```c
#include "repeat.h"

void repeat(const char *s, int times, char *buf, size_t n)
{
    size_t pos = 0;

    for (int i = 0; i < times; i++) {
        for (size_t j = 0; s[j] != '\0' && pos + 1 < n; j++) {
            buf[pos] = s[j];
            pos++;
        }
    }

    if (n > 0) {
        buf[pos] = '\0';
    }
}
```

Run the tests. Green.

### A word on `for` versus `while`

Anything you can write with `for` you can write with `while`:

```c
int i = 0;
while (i < times) {
    ...
    i++;
}
```

The convention in C is to use `for` when the loop is driven by a counter, and
`while` when it is driven by a condition you cannot neatly express as three
clauses. You will see both in the standard library's headers. Pick whichever
makes the loop read more clearly.

## Refactor

Nothing jumps out. But there is a subtle bug hiding in the test: we assumed
`buf` is big enough for the whole result. What if it is not? A good test to
add is one that deliberately passes a tiny buffer and checks we truncate
rather than overflow:

```c
static void test_repeat_truncates_to_fit(void)
{
    char buf[4];

    repeat("ab", 5, buf, sizeof buf);

    CHECK_STR(buf, "aba");
}
```

Run it under the sanitizer build and it will stay quiet, because the loop's
condition is what stops us. If you had written `buf[pos] = s[j]` without the
`pos + 1 < n` check, AddressSanitizer would have found the overflow for you.
That is the whole point of the sanitizers: the bug you would not have noticed
until a customer did.

## Benchmarking

Go ships with benchmarks: `func BenchmarkRepeat(b *testing.B)`. C does not, so
we write one. A benchmark is just a program that runs something a lot and
times it.

The clock we want is `clock_gettime` with `CLOCK_MONOTONIC` — a clock that
only ever moves forward, unlike the wall clock, which can jump when the system
adjusts the time.

```c
#include <stdio.h>
#include <time.h>

#include "repeat.h"

static double seconds_between(struct timespec start, struct timespec end)
{
    return (double)(end.tv_sec - start.tv_sec) +
           (double)(end.tv_nsec - start.tv_nsec) / 1e9;
}

int main(void)
{
    char buf[1024];
    struct timespec start, end;

    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < 1000000; i++) {
        repeat("ab", 10, buf, sizeof buf);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);

    printf("repeat x 1,000,000 took %.3f ms\n",
           seconds_between(start, end) * 1000.0);
    return 0;
}
```

Build it in release mode and run it:

```sh
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release --target iteration_v2_bench
./build-release/iteration/iteration_v2_bench
```

```text
repeat x 1,000,000 took 8.412 ms
```

Your number will differ; that is fine. What matters is that you now have a way
to answer "did that change make it faster?" before you tell anyone it did.

> **Measure, do not guess.** C compilers are extremely good at optimising, and
> humans are extremely bad at predicting which line is slow. A benchmark is
> five minutes of work and it beats an argument every time.

### Additional material

- [Godbolt Compiler Explorer](https://godbolt.org/) — paste your code and see the assembly
- [`clock_gettime` man page](https://man7.org/linux/man-pages/man3/clock_gettime.3.html)
