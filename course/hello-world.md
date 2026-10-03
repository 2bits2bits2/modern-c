# Hello, World

**[You can find all the code for this chapter here](hello-world/)**

It is traditional for your first program in a new language to be
[Hello, World](https://en.m.wikipedia.org/wiki/%22Hello,_World!%22_program).

In Go, you write a test and the language hands you a testing framework. In C,
there is no `testing` package. There is no `t.Errorf`. If we want to write
tests — and we do — we have to make a tiny harness ourselves. That is a
feature, not a bug: you are about to understand every line of your testing
library.

## Our test harness

The book ships a harness called **mctest** in the `test/` directory. Here is
its whole interface:

```c
void run_test(const char *name, void (*fn)(void));

void check_str(const char *got, const char *want, const char *file, int line);
void check_int(long long got, long long want, const char *file, int line);
void check_double(double got, double want, double tolerance, const char *file, int line);
void check_true(int condition, const char *expr, const char *file, int line);
void check_fail(const char *file, int line, const char *fmt, ...);

int test_summary(void);

#define RUN_TEST(fn) run_test(#fn, fn)
#define HERE __FILE__, __LINE__
#define CHECK_STR(got, want) check_str((got), (want), HERE)
#define CHECK_INT(got, want) check_int((got), (want), HERE)
#define CHECK_TRUE(cond) check_true((cond), #cond, HERE)
#define FAIL(...) check_fail(HERE, __VA_ARGS__)
```

And here is the interesting part of the implementation, so you can see there
is no magic:

```c
void run_test(const char *name, void (*fn)(void))
{
    int failed_before = checks_failed;

    current_test = name;
    tests_run++;
    fn();

    if (checks_failed == failed_before) {
        printf("--- PASS %s\n", name);
    } else {
        tests_failed++;
        printf("--- FAIL %s\n", name);
    }
}

void check_str(const char *got, const char *want, const char *file, int line)
{
    checks_run++;

    if (got == want) {
        return;
    }
    if (got != NULL && want != NULL && strcmp(got, want) == 0) {
        return;
    }

    report(file, line, "got \"%s\" want \"%s\"", got ? got : "(null)",
           want ? want : "(null)");
}
```

A few things worth noticing:

- `RUN_TEST(fn)` uses the preprocessor stringify operator `#fn` to turn the
  function name into the test's name. That is how `test_hello` becomes
  `"test_hello"` in the output.
- `CHECK_STR` passes `__FILE__` and `__LINE__`, so a failure tells you exactly
  where it happened. This is our `t.Errorf`.
- We keep running after a failed check rather than aborting, so one run can
  report every problem at once.

Read `test/mctest.c` when you get a moment. It is about a hundred lines and
you will use it for the rest of the book.

## How a C program is put together

Before the test, the code. Create a file called `hello.c`:

```c
#include "hello.h"

const char *hello(void)
{
    return "Hello, world";
}
```

And `hello.h`, its declaration:

```c
#ifndef HELLO_H
#define HELLO_H

const char *hello(void);

#endif /* HELLO_H */
```

C separates *declaration* from *definition*. The header says "there is a
function called `hello`, it takes no arguments and returns a `const char *`".
The `.c` file says what it actually does. Any file that wants to call `hello`
includes `hello.h`; the compiler then knows the function's shape, and the
*linker* later connects the call to the definition.

The `#ifndef HELLO_H` is an **include guard**. It stops the header being
included twice in one translation unit, which would be a redefinition error.
Every header in this book will have one.

Let's write a program that uses it, in `main.c`:

```c
#include <stdio.h>

#include "hello.h"

int main(void)
{
    printf("%s\n", hello());
    return 0;
}
```

`printf` is our `fmt.Println`, from `<stdio.h>`. `%s` is the format verb for a
string.

## Write the test first

Now the test, in `hello_test.c`:

```c
#include "hello.h"
#include "mctest.h"

static void test_hello(void)
{
    CHECK_STR(hello(), "Hello, world");
}

int main(void)
{
    RUN_TEST(test_hello);
    return test_summary();
}
```

A test binary is just a program with a `main`. `main` runs each test and then
returns `test_summary()`, which returns `0` when everything passed and `1`
otherwise. That non-zero exit code is what `ctest` looks at.

`CHECK_STR` is the string comparison. We write the *want* second, because
that reads like the sentence "got X, want Y" you will see when it fails.

## Try to run the test

```sh
cmake --build build
ctest --test-dir build --output-on-failure
```

You should see it pass:

```text
--- PASS test_hello

1 test(s), 1 check(s), 0 failed
PASS
```

Now the important habit: if you have never seen this test *fail*, you do not
know that it tests anything. Change the `want` string to `"Goodbye, world"`,
run it, and read the failure:

```text
    hello-world/v1/hello_test.c:6: test_hello: got "Hello, world" want "Goodbye, world"
--- FAIL test_hello

1 test(s), 1 check(s), 1 failed
FAIL
```

That is an easy-to-understand description of what is wrong. Put the string
back and make sure you are green again before moving on.

## A note on source control

At this point, if you are using source control (which you should!), commit the
code as it is. We have working software backed by a test. I would not push to
share it just yet, because we are about to change the design — but a commit
here gives you something solid to come back to if a refactor goes sideways.

## Hello, YOU

Our next requirement is to greet a specific person: `hello("Chris")` should
produce `Hello, Chris`.

But wait. In Go this is easy, because a function can just return a fresh
string. In C, a string is a `char` array, and `hello` has nowhere to put a
newly built string. It could return a pointer to a static buffer, but that is
a trap: two calls would fight over the same memory.

The honest C answer is to let the *caller* own the memory and pass in a
buffer:

```c
void hello(const char *name, char *buf, size_t n);
```

`buf` is where we write the result, and `n` is how many bytes are available.
This is a very common shape in C: you will see `(char *buf, size_t n)` all
over the standard library.

Let's capture the new requirement in a test first.

```c
static void test_hello_to_a_person(void)
{
    char buf[64];

    hello("Chris", buf, sizeof buf);

    CHECK_STR(buf, "Hello, Chris");
}
```

## Try to run the test

We have written the test as if the new `hello` already existed, but the header
still declares the old one. Run the build and read the compiler:

```text
hello_test.c: In function ‘test_hello_to_a_person’:
hello_test.c:6:37: error: too many arguments to function ‘hello’
    6 |     hello("Chris", buf, sizeof buf);
      |                                     ^~~~~
hello.h:3:13: note: declared here
    3 | const char *hello(void);
      |             ^~~~~
```

Listen to the compiler. It is telling us, precisely, that the declaration and
the call do not agree. Update `hello.h`:

```c
#include <stddef.h>

void hello(const char *name, char *buf, size_t n);
```

We need `<stddef.h>` for `size_t`. Now the *implementation* no longer matches
the header, and the compiler tells us that too:

```text
hello.c: In function ‘hello’:
hello.c:3:6: error: conflicting types for ‘hello’; have ‘const char *(void)’
```

This is C's version of a type error, and it is your friend. One declaration,
one definition, and the compiler keeps them honest.

## Write the minimal amount of code to make the test pass and check the failing test output

Change `hello.c` to match the new signature, but do the *least* possible work:

```c
#include "hello.h"

#include <stdio.h>

void hello(const char *name, char *buf, size_t n)
{
    (void)name;
    snprintf(buf, n, "Hello, world");
}
```

`(void)name;` is a small idiom that says "I know this parameter is unused" —
without it, `-Wextra` will complain.

`snprintf(buf, n, ...)` is the safe string builder: it writes at most `n`
bytes including the terminating `'\0'`. Fortune favours the careful.

Run the test. It compiles, and now it fails for the *right* reason:

```text
    hello-world/v2/hello_test.c:6: test_hello_to_a_person: got "Hello, world" want "Hello, Chris"
--- FAIL test_hello_to_a_person
```

Good. The test is doing its job.

## Write enough code to make it pass

```c
void hello(const char *name, char *buf, size_t n)
{
    snprintf(buf, n, "Hello, %s", name);
}
```

Run the tests. Green.

## Refactor

Not much to do yet. One thing we can improve is that `main.c` no longer
compiles, because it calls the old `hello()`. This is a nice illustration of
why the compiler is valuable: it found every caller we forgot. Update it:

```c
int main(void)
{
    char buf[64];

    hello("world", buf, sizeof buf);
    puts(buf);
    return 0;
}
```

`puts` writes a string followed by a newline, which is exactly what we want
here.

## Hello, world... again

The next requirement: when the name is empty, default to `"World"` rather than
writing a trailing space.

Test first:

```c
static void test_empty_name_defaults_to_world(void)
{
    char buf[64];

    hello("", buf, sizeof buf);

    CHECK_STR(buf, "Hello, World");
}
```

Run it and confirm the failure is meaningful:

```text
    hello-world/v3/hello_test.c:6: test_empty_name_defaults_to_world: got "Hello, " want "Hello, World"
--- FAIL test_empty_name_defaults_to_world
```

Now fix it with an `if`:

```c
void hello(const char *name, char *buf, size_t n)
{
    if (name[0] == '\0') {
        name = "World";
    }

    snprintf(buf, n, "Hello, %s", name);
}
```

`name[0] == '\0'` is how C asks "is this string empty?". There is no `==` for
strings, only for single characters, which is why we compare the first one
against the null terminator.

Run the tests; both pass.

### Refactoring our tests

Our tests repeat the same three lines: declare a buffer, call, check. In Go
you would reach for a helper. In C the natural helper is a function, but we
need it to report into *our* test, so it takes the file and line of the
caller — or, more simply, we just let it use `CHECK_STR`, which already
captures them:

```c
static void assert_hello(const char *name, const char *language,
                         const char *want)
{
    char buf[64];

    hello(name, language, buf, sizeof buf);
    CHECK_STR(buf, want);
}
```

Then `test_hello_to_a_person` becomes a one-liner. We will introduce this in
the next step, once `hello` takes a language too.

## More requirements: languages

We now need a second parameter, specifying the language. If a language is
passed in that we do not recognise, default to English.

Test first:

```c
static void test_in_spanish(void)
{
    char buf[64];

    hello("Elodie", "Spanish", buf, sizeof buf);

    CHECK_STR(buf, "Hola, Elodie");
}
```

The compiler complains again — `hello` takes three arguments, not four. Add
the parameter, then make the test pass the smallest way you can, and watch it
fail with the right message:

```text
    hello-world/v4/hello_test.c:6: test_in_spanish: got "Hello, Elodie" want "Hola, Elodie"
```

Now write enough to pass. Because we are going to add more languages, let's
put the decision in one place:

```c
static const char *greeting_prefix(const char *language)
{
    if (strcmp(language, "Spanish") == 0) {
        return "Hola, ";
    }
    if (strcmp(language, "French") == 0) {
        return "Bonjour, ";
    }
    return "Hello, ";
}

void hello(const char *name, const char *language, char *buf, size_t n)
{
    if (name[0] == '\0') {
        name = "World";
    }

    snprintf(buf, n, "%s%s", greeting_prefix(language), name);
}
```

`strcmp(a, b) == 0` is how C compares strings. It returns `0` when they are
equal, which trips everyone up at least once. There is no `==` for strings.

Run the tests. All green. Now refactor the tests to use the helper we sketched:

```c
static void assert_hello(const char *name, const char *language,
                         const char *want)
{
    char buf[64];

    hello(name, language, buf, sizeof buf);
    CHECK_STR(buf, want);
}
```

And the tests become specifications:

```c
static void test_hello_to_a_person(void)
{
    assert_hello("Chris", "", "Hello, Chris");
}

static void test_empty_name_defaults_to_world(void)
{
    assert_hello("", "", "Hello, World");
}

static void test_in_spanish(void)
{
    assert_hello("Elodie", "Spanish", "Hola, Elodie");
}

static void test_in_french(void)
{
    assert_hello("Lauren", "French", "Bonjour, Lauren");
}

static void test_unknown_language_falls_back_to_english(void)
{
    assert_hello("Chris", "Klingon", "Hello, Chris");
}
```

Add a test for a language of your own and see how little code it takes to
extend `hello`.

## Wrapping up

Who knew you could get so much out of Hello, World? By now you should have
some understanding of:

### C syntax around

- Header files, include guards, declarations and definitions
- `main` returning an exit code
- Functions with parameters and a return type, and `(char *buf, size_t n)`
- C strings: they are `char` arrays, `""` is an empty string, `strcmp` for
  equality, `snprintf` for building one
- `if` and `const`
- `static`, and why helper functions in a `.c` file are usually `static`

### The TDD process

- *Write a failing test and see it fail*, so we know the test is relevant and
  that its failure message is readable
- Writing the smallest amount of code to make it pass
- *Then* refactoring, backed by tests

### The C flavour of the loop

- The compiler error is the first checkpoint, not an obstacle
- Nothing is hidden: the test harness, the buffer sizes and the string
  handling are all code you can read

You will not get a `testing` package in C. But by the end of this book you
will not miss one, either.
