# Arrays and pointers

**[You can find all the code for this chapter here](arrays-and-pointers/)**

We need a `sum` function that adds up a list of numbers. That sounds trivial,
and the syntax is — but it is also where C's most important idea lives: an
array is not a thing you pass around, it is a pointer and a length, and the
two are separate.

## Write the test first

```c
#include "sum.h"
#include "mctest.h"

static void test_sum_of_numbers(void)
{
    int numbers[] = {1, 2, 3, 4, 5};

    CHECK_INT(sum(numbers, 5), 15);
}

int main(void)
{
    RUN_TEST(test_sum_of_numbers);
    return test_summary();
}
```

## Try to run the test

```text
arrays-and-pointers/v1/sum_test.c:3:31: error: implicit declaration of function ‘sum’ [-Werror=implicit-function-declaration]
cc1: all warnings being treated as errors
```

The usual. The header:

```c
#include <stddef.h>

long sum(const int *values, size_t n);
```

Look carefully at that parameter list, because it is the heart of the chapter.
We are not passing "an array". We are passing a **pointer** to the first
element, and a **length**. `sizeof(values)` inside `sum` would give the size of
a pointer — usually 8 bytes — not the size of the array. The information about
how long the array is only exists where the array was declared, and it does
not travel with the pointer. So we pass it explicitly.

> In Go you can call `len(s)` on a slice because the slice carries its length
> with it. A C array is just the address of the first element. Losing the
> length is the single most common C mistake, so we make it a habit: any
> function that takes an array takes a count as well.

## Write the minimal amount of code for the test to run and check the failing test output

```c
#include "sum.h"

long sum(const int *values, size_t n)
{
    (void)values;
    (void)n;
    return 0;
}
```

```text
    arrays-and-pointers/v1/sum_test.c:8: test_sum_of_numbers: got 0 want 15
--- FAIL test_sum_of_numbers
```

## Write enough code to make it pass

```c
long sum(const int *values, size_t n)
{
    long total = 0;

    for (size_t i = 0; i < n; i++) {
        total += values[i];
    }

    return total;
}
```

`values[i]` is shorthand for `*(values + i)`: index into the pointer, then
dereference. It is the same operation, and you will see both spellings in real
code.

`total` is a `long` rather than an `int`. Adding up a lot of large `int`s can
overflow an `int`, and as we learned in the [integers](integers.md) chapter,
signed overflow is undefined behaviour. Widening the accumulator is a cheap
insurance policy.

## Refactor

Now that it works, let's think about the edge case. What should `sum` return
for an empty list? Zero, obviously. But what does the *caller* pass? A pointer
that points at nothing and a length of zero.

```c
static void test_sum_of_empty_array(void)
{
    CHECK_INT(sum(NULL, 0), 0);
}
```

Our loop never dereferences `values`, so `NULL` is safe here. This is a good
test to have because it documents the contract: `values` may be `NULL` as long
as `n` is zero. Without the test, some future refactor could dereference it
unconditionally and nobody would notice until it crashed.

## More requirements: a growable list

Product now wants to build a list a piece at a time, without knowing the size
up front. In Go you would reach for a slice and `append`. In C, a slice is a
`struct` we write ourselves: a pointer to some heap memory, how much we are
using, and how much we have.

This is the chapter where we finally meet `malloc`, `realloc` and `free`.

### Just enough heap

- `malloc(n)` asks the operating system for `n` bytes and returns a pointer to
  them, or `NULL` if it cannot
- `realloc(p, n)` resizes a block you got from `malloc`, copying the contents
  if it has to move, and returns the new pointer (or `NULL`, leaving the old
  block alone)
- `free(p)` gives the memory back
- Every `malloc` needs exactly one `free`. Nothing does it for you. No garbage
  collector is coming.

The classic mistake is `p = realloc(p, n)`: if `realloc` fails and returns
`NULL`, you have just overwritten your only pointer to the old block, leaking
it. Always keep the result in a temporary.

### Write the test first

```c
#include "ints.h"
#include "mctest.h"

static void test_append_single(void)
{
    struct ints xs = ints_new();

    CHECK_TRUE(ints_append(&xs, 7));
    CHECK_INT(xs.len, 1);
    CHECK_INT(xs.data[0], 7);

    ints_free(&xs);
}
```

Notice `ints_free(&xs)` at the end. Every test that allocates must free, or the
sanitizer build will tell on you.

### Write enough code to make it pass

The header describes the slice:

```c
struct ints {
    int *data;
    size_t len;
    size_t cap;
};

struct ints ints_new(void);
void ints_free(struct ints *xs);
bool ints_append(struct ints *xs, int value);
```

And the implementation:

```c
struct ints ints_new(void)
{
    return (struct ints){.data = NULL, .len = 0, .cap = 0};
}

void ints_free(struct ints *xs)
{
    free(xs->data);
    xs->data = NULL;
    xs->len = 0;
    xs->cap = 0;
}

static bool ints_grow(struct ints *xs, size_t needed)
{
    if (needed <= xs->cap) {
        return true;
    }

    size_t new_cap = xs->cap == 0 ? 4 : xs->cap * 2;
    while (new_cap < needed) {
        new_cap *= 2;
    }

    int *new_data = realloc(xs->data, new_cap * sizeof *new_data);
    if (new_data == NULL) {
        return false;
    }

    xs->data = new_data;
    xs->cap = new_cap;
    return true;
}

bool ints_append(struct ints *xs, int value)
{
    if (!ints_grow(xs, xs->len + 1)) {
        return false;
    }

    xs->data[xs->len] = value;
    xs->len++;
    return true;
}
```

A few things worth staring at:

- `(struct ints){...}` is a **compound literal** — it builds a struct value
  right there. Our zero-capacity list is honest: no data, no length, no
  capacity.
- `new_cap * sizeof *new_data` is the amount of memory for `new_cap` ints.
  `sizeof *new_data` means "the size of the thing `new_data` points at", and it
  is immune to the type changing later. Prefer it to `sizeof(int)`.
- On failure we return `false` and leave the list untouched. The caller can
  decide what to do; we do not leak.

Add a test that appends more than the initial capacity, so the grow path runs:

```c
static void test_append_many_grows(void)
{
    struct ints xs = ints_new();

    for (int i = 0; i < 100; i++) {
        CHECK_TRUE(ints_append(&xs, i));
    }

    CHECK_INT(xs.len, 100);
    for (int i = 0; i < 100; i++) {
        CHECK_INT(xs.data[i], i);
    }

    ints_free(&xs);
}
```

Run it. Then run it again under the sanitizers:

```sh
cmake -S . -B build-asan -DENABLE_SANITIZERS=ON
cmake --build build-asan
ctest --test-dir build-asan --output-on-failure
```

If you forget an `ints_free`, AddressSanitizer will name the function and the
byte count of the leak. This is a superpower that Go developers only get
because the runtime does it for them; in C you have to turn it on, but you do
get it.

## Wrapping up

What we have covered:

- Arrays decay to pointers, so a length must travel alongside
- `values[i]` is `*(values + i)`
- `NULL` with a length of zero is a reasonable empty array
- The heap: `malloc`, `realloc`, `free`, and the golden rule of one `free` per
  `malloc`
- The slice pattern: a struct with `data`, `len` and `cap`, and geometric
  growth

In Go, all of this is one keyword: `append`. In C it is a small amount of code
that you now understand completely — which is why you will be able to debug it.

### Additional material

- [`realloc` man page](https://man7.org/linux/man-pages/man3/realloc.3.html) — worth reading in full, twice
