# Integers

**[You can find all the code for this chapter here](integers/)**

Integers work as you would expect. Let's write an `add` function to try things
out, and along the way meet a genuinely modern C feature: checked arithmetic.

## Write the test first

Create `adder_test.c`:

```c
#include "adder.h"
#include "mctest.h"

static void test_add(void)
{
    CHECK_INT(add(2, 2), 4);
}

int main(void)
{
    RUN_TEST(test_add);
    return test_summary();
}
```

Notice we switched from `CHECK_STR` to `CHECK_INT`. We want the failure to
print integers, not strings, and we want it to compare numerically.

## Try to run the test

```sh
cmake --build build
```

The compiler has not seen `add`:

```text
adder_test.c: In function ‘test_add’:
adder_test.c:6:32: error: implicit declaration of function ‘add’ [-Werror=implicit-function-declaration]
    6 |     CHECK_INT(add(2, 2), 4);
      |                                ^~~
cc1: all warnings being treated as errors
```

In older C this was merely a warning and the compiler assumed `add` returned
`int`. C23 makes implicit declarations an error, and with `-Werror` we see it
loud and clear. The fix is a declaration — a header file.

## Write the minimal amount of code for the test to run and check the failing test output

`adder.h`:

```c
#ifndef ADDER_H
#define ADDER_H

int add(int a, int b);

#endif /* ADDER_H */
```

`adder.c`:

```c
#include "adder.h"

int add(int a, int b)
{
    return 0;
}
```

Run the test and check the failure says what we think it says:

```text
    integers/v1/adder_test.c:6: test_add: got 0 want 4
--- FAIL test_add
```

## Write enough code to make it pass

```c
int add(int a, int b)
{
    return a + b;
}
```

Run the tests. Green.

## Refactor

There is not much in the *implementation* to improve. But this is a good
moment to talk about documentation, because in C it does not come for free.

Go has `go doc` and generates documentation from comments. C has no such
thing in the language, but the convention is well established: a comment above
a declaration, in the header, using [Doxygen](https://www.doxygen.nl/) style.
That is where people look.

```c
/**
 * add returns the sum of a and b.
 *
 * The result is undefined if the true sum does not fit in an int.
 */
int add(int a, int b);
```

Writing that comment immediately exposes a problem we have been ignoring:
what happens on overflow? Let's find out.

## Property based thinking, and the limits of `int`

In Go, an `int` is 64 bits on most machines and signed overflow wraps. In C,
signed integer overflow is **undefined behaviour**. That is not a scary word
for no reason — the compiler is allowed to assume it never happens, and an
optimisation can turn a "harmless" overflow into a very surprising program.

We cannot test undefined behaviour reliably, so we should not let it happen.
C gives us a way to detect it *before* it occurs:

```c
#include <stdbool.h>

bool add_checked(int a, int b, int *result);
```

It returns `true` and writes the sum to `*result` when the addition fits, and
returns `false` without touching `*result` when it does not.

## Write the test first

```c
#include <limits.h>

#include "adder.h"
#include "mctest.h"

static void test_add_checked_within_range(void)
{
    int got = 0;

    CHECK_TRUE(add_checked(2, 2, &got));
    CHECK_INT(got, 4);
}

static void test_add_checked_detects_overflow(void)
{
    int got = 0;

    CHECK_TRUE(!add_checked(INT_MAX, 1, &got));
}
```

`INT_MAX` from `<limits.h>` is the largest representable `int`. Adding one to
it is exactly the case we care about.

## Write enough code to make it pass

The temptation is to check the sign of the result, but that is fiddly and
easy to get wrong. Use the compiler's builtin instead:

```c
bool add_checked(int a, int b, int *result)
{
    return !__builtin_add_overflow(a, b, result);
}
```

`__builtin_add_overflow` comes from GCC and Clang. It computes the addition at
full width and reports whether the result fits in the destination type. It is
fast — usually a single instruction — and it is correct on every edge case,
including the ones that are hard to reason about by hand.

Add a test for underflow too, and make sure it passes:

```c
static void test_add_checked_detects_underflow(void)
{
    int got = 0;

    CHECK_TRUE(!add_checked(INT_MIN, -1, &got));
}
```

## Wrapping up

What we have covered:

- More TDD practice, including a second, richer requirement
- Integers, `int`, and `size_t`-free arithmetic for now
- Header files as the home of documentation
- Signed overflow is undefined behaviour, and how to avoid it with
  `__builtin_add_overflow`

### Additional material

- [A Guide to Undefined Behavior in C and C++](https://blog.regehr.org/archives/213)
- [GCC documentation for integer overflow builtins](https://gcc.gnu.org/onlinedocs/gcc/Integer-Overflow-Builtins.html)
