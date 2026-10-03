# Generics

**[You can find all the code for this chapter here](generics/)**

Go 1.18 added generics. C has had a form of them since 2011 (`_Generic`) and
has always had the other form (the preprocessor), and C23 sharpened both with
`typeof` and better `_Generic`. In this chapter we write a `max` that picks its
own type, and then a macro that generates a whole typed collection.

## `_Generic`: choose a function by type

We want `max(a, b)` to work for `int` and `double`. Write one function per
type, then dispatch:

```c
static inline int max_int(int a, int b)
{
    return a > b ? a : b;
}

static inline double max_double(double a, double b)
{
    return a > b ? a : b;
}

#define max(a, b)                                                              \
    _Generic((a), int : max_int, double : max_double, default : max_int)((a),  \
                                                                         (b))
```

The macro expands to a call to whichever function matches the type of `a`. The
compiler then type-checks the call, so `max(1.5, 2)` uses `max_double` and
widens `2`, while `max(some_string, 2)` is a compile error.

`typeof` (C23) is the companion feature. It gives you the type of an
expression, which lets a macro declare a temporary of the right type — as in
this generic `swap`:

```c
#define swap(a, b)                                                             \
    do {                                                                       \
        typeof(a) mctest_tmp = (a);                                            \
        (a) = (b);                                                             \
        (b) = mctest_tmp;                                                      \
    } while (0)
```

Three things are going on here that make macros *safe*:

- `typeof(a)` declares a temporary with exactly the type of the left operand
- wrapping in `do { ... } while (0)` makes the macro behave like a single
  statement, so it works correctly after an `if` without braces
- evaluating each argument once — `a` and `b` appear on both sides, so if they
  were function calls they would each be called twice. Naming the temporary
  avoids that for `a`, and you would need a second temporary for `b` if the
  arguments might be expensive.

The whole point of these tricks is that the macro is *invisible* at the call
site: `swap(x, y)` behaves like a function.

### Write the test first

```c
static void test_max_picks_the_type(void)
{
    CHECK_INT(max(3, 7), 7);
    CHECK_DOUBLE(max(2.5, 1.5), 2.5, 1e-9);
}

static void test_swap(void)
{
    int a = 1;
    int b = 2;

    swap(a, b);

    CHECK_INT(a, 2);
    CHECK_INT(b, 1);
}
```

## Generating a collection

`_Generic` picks between *existing* functions. The much bigger use of the
preprocessor is generating those functions in the first place. You have
probably already noticed how similar `struct int_array` and
`struct double_array` would be. Let us write that array once, for any type:

```c
#define DEFINE_ARRAY(NAME, TYPE)                                               \
    struct NAME {                                                              \
        TYPE *data;                                                            \
        size_t len;                                                            \
        size_t cap;                                                            \
    };                                                                         \
                                                                               \
    static inline void NAME##_init(struct NAME *a)                             \
    {                                                                          \
        a->data = NULL;                                                         \
        a->len = 0;                                                            \
        a->cap = 0;                                                            \
    }                                                                          \
                                                                               \
    static inline bool NAME##_push(struct NAME *a, TYPE value)                 \
    {                                                                          \
        if (a->len == a->cap) {                                                \
            size_t cap = a->cap == 0 ? 4 : a->cap * 2;                         \
            TYPE *data = realloc(a->data, cap * sizeof *data);                 \
            if (data == NULL) {                                                \
                return false;                                                  \
            }                                                                  \
            a->data = data;                                                    \
            a->cap = cap;                                                      \
        }                                                                      \
        a->data[a->len] = value;                                               \
        a->len++;                                                              \
        return true;                                                           \
    }
```

The `##` operator pastes tokens together: if `NAME` is `int_array`, then
`NAME##_push` becomes `int_array_push`. The backslash at the end of every line
is the preprocessor's line continuation; without it, the macro ends at the end
of the first line.

Now a caller does this:

```c
DEFINE_ARRAY(int_array, int)
DEFINE_ARRAY(double_array, double)
```

and gets `struct int_array`, `int_array_init`, `int_array_push`,
`int_array_free`, and the same for doubles. The generated code is ordinary C
that you could have typed by hand — because it *is* ordinary C.

### Write the test first

```c
static void test_int_array(void)
{
    struct int_array a;

    int_array_init(&a);
    for (int i = 0; i < 10; i++) {
        CHECK_TRUE(int_array_push(&a, i));
    }

    CHECK_INT(a.len, 10);
    int_array_free(&a);
}
```

## The honest trade-offs

Macros as generics are powerful, but they are not free:

- **Error messages are terrible.** A mistake inside `DEFINE_ARRAY` is reported
  at the line where you *used* it, not where you wrote it. `gcc -E` shows you
  the preprocessed code, which is the antidote.
- **Debuggers step through expanded code**, which can be confusing. Compile
  with `-g` and know that breakpoints may land on strange lines.
- **Code size.** Each `DEFINE_ARRAY` is a separate copy. For small helpers that
  is fine; for a huge function it matters.
- **No type-checking inside the macro** until it is instantiated, which means
  the macro's "contract" is whatever happens to compile.

Compare that to Go generics, which the compiler understands natively and can
check and document. C's answer is the preprocessor, and the reason it persists
is that the generated code is exactly as fast and as debuggable as hand-written
C — because it *is* hand-written C, just written by the preprocessor.

## Wrapping up

What we have covered:

- `_Generic` for compile-time dispatch between existing functions
- `typeof` for declaring a temporary of an argument's type
- `do { } while (0)` and the single-evaluation rule for safe macros
- `##` token pasting and line continuation in macros
- `DEFINE_ARRAY`: generating a typed collection from one definition
- The real trade-offs: error messages, debuggers, code size

This is the most powerful thing in the C preprocessor, and the most dangerous.
Used with restraint — small, well-named, single-purpose macros — it lets you
write collections and algorithms once instead of once per type. Used without
restraint, it creates code nobody can debug.

### Additional material

- [`_Generic` reference](https://en.cppreference.com/w/c/language/generic)
- [C23 `typeof`](https://en.cppreference.com/w/c/language/typeof)
- [C preprocessor tricks](https://github.com/pfultz2/Clang)

