# Structs, function pointers & interfaces

**[You can find all the code for this chapter here](structs-and-function-pointers/)**

Product wants us to calculate the area and perimeter of shapes. To start, a
rectangle. This is the chapter where C's lack of objects becomes interesting,
because the answers are *structs* and *function pointers*, and together they
give you something that behaves a lot like Go's interfaces.

## Write the test first

```c
#include "mctest.h"
#include "rectangle.h"

static void test_area(void)
{
    struct rectangle r = {.width = 3, .height = 4};

    CHECK_DOUBLE(rectangle_area(r), 12, 1e-9);
}
```

`CHECK_DOUBLE` takes a third argument: a tolerance. Floating point numbers are
rarely exactly equal, so we compare with a little slack. `1e-9` is tight
enough for our purposes.

## Try to run the test

```text
structs-and-function-pointers/v1/rectangle_test.c:8:59: error: implicit declaration of function ‘rectangle_area’ [-Werror=implicit-function-declaration]
```

The header:

```c
struct rectangle {
    double width;
    double height;
};

double rectangle_area(struct rectangle r);
double rectangle_perimeter(struct rectangle r);
```

A `struct` is a bundle of values under one name. Unlike a Go `struct`, there
are no methods attached to it — C functions take the struct as an argument,
and where you would write `r.Area()` you write `rectangle_area(r)`.

## Write the minimal amount of code for the test to run and check the failing test output

```c
double rectangle_area(struct rectangle r)
{
    (void)r;
    return 0;
}
```

```text
    structs-and-function-pointers/v1/rectangle_test.c:8: test_area: got 0 want 12 (tolerance 1e-09)
--- FAIL test_area
```

## Write enough code to make it pass

```c
double rectangle_area(struct rectangle r)
{
    return r.width * r.height;
}

double rectangle_perimeter(struct rectangle r)
{
    return 2 * (r.width + r.height);
}
```

## Refactor: taking the struct by value

`rectangle_area` takes `struct rectangle` by value, so the whole struct is
copied on every call. For two `double`s that is fine. For a large struct it is
wasteful, and the convention in C is to pass a **pointer** instead:

```c
double rectangle_area(const struct rectangle *r);
```

The `const` is a promise that we will not modify what `r` points at. It is
both documentation and something the compiler will enforce. We will leave the
by-value version in this chapter because it reads well for tiny structs, but
expect to see `const T *` everywhere in real code.

## More requirements: a circle

A rectangle's area is `width * height`; a circle's is `πr²`. We could write
`circle_area`, but then every new shape needs a new function and every bit of
code that consumes shapes needs to know which function to call.

Go solves this with an interface:

```go
type Shape interface {
    Area() float64
}
```

C has no `interface` keyword. But it has function pointers, and a function
pointer in a struct is exactly what an interface compiles down to anyway.

### Write the test first

We want one function, `shape_area`, that works for any shape:

```c
static void test_shapes(void)
{
    struct rectangle r = {.width = 3, .height = 4};
    struct circle c = {.radius = 2};

    struct shape_case cases[] = {
        {.shape = rectangle_as_shape(&r), .want_area = 12, .want_perimeter = 14},
        {.shape = circle_as_shape(&c), .want_area = PI * 4, .want_perimeter = 4 * PI},
    };

    size_t n = sizeof cases / sizeof cases[0];
    for (size_t i = 0; i < n; i++) {
        CHECK_DOUBLE(shape_area(cases[i].shape), cases[i].want_area, 1e-9);
        CHECK_DOUBLE(shape_perimeter(cases[i].shape), cases[i].want_perimeter, 1e-9);
    }
}
```

This is a **table-driven test**. Adding a new shape means adding a row, not a
new test function. In C the table is just an array of structs, and
`sizeof cases / sizeof cases[0]` is the idiom for "how many rows" — one of the
few places C does give you a length for free, because `cases` is a real array
here, not a pointer.

### Write enough code to make it pass

The "interface" is two pointers to functions plus a pointer to the value:

```c
struct shape_vtable {
    double (*area)(const void *self);
    double (*perimeter)(const void *self);
};

struct shape {
    const struct shape_vtable *vtable;
    const void *self;
};
```

`double (*area)(const void *self)` reads as: `area` is a pointer to a function
taking a `const void *` and returning a `double`. The parentheses matter —
without them, `double *area(const void *)` would mean "a function returning a
pointer to double", which is a different and much less useful thing.

`const void *self` is a type-erased pointer: it can point at a rectangle, a
circle, or anything else. That is the same trick Go uses under the hood for
interface values, minus the safety.

Each concrete type gets a `static` vtable and small adapter functions:

```c
static double rectangle_area(const void *self)
{
    const struct rectangle *r = self;
    return r->width * r->height;
}

static const struct shape_vtable rectangle_vtable = {
    .area = rectangle_area,
    .perimeter = rectangle_perimeter,
};
```

And the generic entry points dispatch through the vtable:

```c
double shape_area(struct shape s)
{
    return s.vtable->area(s.self);
}
```

Constructing a shape pairs the value with its vtable:

```c
struct shape rectangle_as_shape(const struct rectangle *r)
{
    return (struct shape){.vtable = &rectangle_vtable, .self = r};
}
```

Run the tests. Both rows pass through the same two functions.

### What is missing compared to Go?

Everything, if you are honest. Go's interfaces are checked by the compiler,
carry method sets, and are garbage collected. Our vtable is a manual
convention:

- Nothing stops you pairing a circle with `rectangle_vtable`; the compiler
  will not notice, and it will read a `double` at the wrong offset
- The `self` pointer must outlive the `struct shape` that wraps it — you get a
  dangling pointer if it does not
- Every new "interface" is a new vtable struct and a set of adapters

That is the trade. In exchange, interfaces in C are just data and function
pointers, and you can see exactly what a dynamic dispatch costs.

## Wrapping up

What we have covered:

- `struct` values, designated initialisers, and passing structs by value or by
  `const` pointer
- Function pointers, and how to read their declarations
- **Interfaces as vtables**: a struct of function pointers plus a type-erased
  `self`
- Table-driven tests, and `sizeof arr / sizeof arr[0]`
- The safety Go gives you for free, that we now have to maintain by hand

This is one of those C features that feels primitive until you use it. Many
mature C codebases — the Linux kernel's `file_operations`, any plugin system,
most C standard library "objects" like `FILE *` — are exactly this pattern.

### Additional material

- [Function pointer syntax](https://cdecl.org/) — the site that decodes C declarations for you
- [Linux kernel `file_operations`](https://elixir.bootlin.com/linux/latest/source/include/linux/fs.h) — a real, large vtable
