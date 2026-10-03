# Reflection and `_Generic`

**[You can find all the code for this chapter here](reflection/)**

Go has reflection: at runtime you can ask a value for its type and act on it. C
has almost none of that. What C has is `_Generic`, which does the type
dispatching at *compile* time, and tagged unions, which do at runtime what
little reflection a C program usually needs. This chapter is about both, and
about being honest about the limits.

## `_Generic`: a switch on types

`_Generic` chooses an expression based on the type of its first argument,
before the program even runs:

```c
#define describe(x)                                                            \
    _Generic((x), int : "int", long : "long", double : "double", float : "float", \
             char * : "string", const char * : "string", default : "unknown")
```

### Write the test first

```c
static void test_describe(void)
{
    int i = 42;
    long l = 42;
    double d = 42;
    const char *s = "hello";

    CHECK_STR(describe(i), "int");
    CHECK_STR(describe(l), "long");
    CHECK_STR(describe(d), "double");
    CHECK_STR(describe(s), "string");
    CHECK_STR(describe(1.5f), "float");
}
```

Each branch has to be a valid expression, and the compiler selects exactly one.
There is no runtime cost: `describe(i)` compiles to a pointer to the string
`"int"`.

### Where this is used for real

`_Generic` is how `<tgmath.h>` gives you `sin`, `cos` and `sqrt` that work on
both `float` and `double` without you choosing a suffix. It is how modern
libraries write one macro that behaves correctly for several types. It is also
the foundation of the [generics chapter](generics.md).

Its limitation is the mirror image of its strength: the type is fixed at
compile time. You cannot pass `describe` a value whose type is only known at
runtime, because there is no such thing in C — every expression has one type,
forever.

## Tagged unions: reflection you carry yourself

If you need a value that *can* be several types at runtime — a JSON document, a
configuration setting, a message on a wire — you carry a tag alongside a union:

```c
enum value_kind {
    VALUE_INT,
    VALUE_DOUBLE,
    VALUE_STR,
};

struct value {
    enum value_kind kind;
    union {
        long i;
        double d;
        const char *s;
    };
};
```

The union means all three members share the same memory; only one is valid at
a time. The `kind` field is what tells you which. This is the pattern behind
dynamic languages' values, and behind every data format you will parse.

### Write the test first

```c
static void test_kind_name(void)
{
    const struct value i = {.kind = VALUE_INT, .i = 1};
    const struct value d = {.kind = VALUE_DOUBLE, .d = 1.5};
    const struct value s = {.kind = VALUE_STR, .s = "x"};

    CHECK_STR(value_kind_name(&i), "int");
    CHECK_STR(value_kind_name(&d), "double");
    CHECK_STR(value_kind_name(&s), "string");
}
```

### Write enough code to make it pass

```c
const char *value_kind_name(const struct value *v)
{
    switch (v->kind) {
    case VALUE_INT:
        return "int";
    case VALUE_DOUBLE:
        return "double";
    case VALUE_STR:
        return "string";
    }
    return "unknown";
}
```

Any function that consumes a `struct value` starts with a `switch` on `kind`.
That is the price of dynamic types in a static language: you write the
dispatch yourself, and the compiler checks that you handled every case you
declared.

### The danger

The union does not know which member is live. If a `struct value` has
`kind == VALUE_INT` but you read `.d`, the compiler will happily let you, and
you will read the bits of a `long` as a `double`. This is the same class of
bug as pairing the wrong vtable with the wrong `self` in the
[structs chapter](structs-and-function-pointers.md). In C, the compiler is
your friend when types are static and silent when they are not. Write accessor
functions — `value_as_double`, `value_as_int` — and use them instead of poking
at the union directly.

## Wrapping up

What we have covered:

- `_Generic` as compile-time type dispatch, with no runtime cost
- Why it cannot help with types known only at runtime
- A tagged union: a tag plus a union, plus `switch` to interpret it
- Accessor functions to keep union access in one place
- The general lesson: in C, you are the reflection

Go's `reflect` package is powerful and slow. C's approach is manual and free.
For the vast majority of programs, the manual version is enough — and it is
much easier to reason about.

### Additional material

- [`_Generic` reference](https://en.cppreference.com/w/c/language/generic)
- [`<tgmath.h>`](https://en.cppreference.com/w/c/numeric/tgmath) — type-generic math in the standard library
