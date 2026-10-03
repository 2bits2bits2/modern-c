# Refactoring Checklist

*This chapter is a discussion, not a TDD tutorial. There is no new code — but
read it, and come back to it when the code you have written starts to smell.*

Refactoring is changing the *structure* of code without changing its
*behaviour*. The tests you have written throughout this book are the whole
reason it is safe: if the behaviour is pinned down, you can rearrange the code
freely and let the tests tell you the moment you break something.

In C this matters more than in most languages, because the things that go
wrong when structure is bad are sharp: a `free` in the wrong place, a pointer
that outlives its buffer, a missed error path that leaks. Good structure in C
is largely good *ownership* structure.

## When to refactor

- After the tests are green. Never refactor on red — you will not know whether
  a failure is the new bug or the old one.
- In small steps, running the tests after each one.
- As a separate activity from changing behaviour. Do not add a feature and
  refactor in the same step; do one, run the tests, then the other.

## A checklist

### Comments

A comment that explains *what* the code does is often a function waiting to be
extracted. `/* check whether the wallet can afford this */` is better expressed
as a function called `wallet_can_afford`. Then the comment is unnecessary, and
the name can be used everywhere.

Comments are still valuable for *why*: why this hack, why this order, why this
magic number. Keep those.

### Names

Names should reveal intent. `int d;` tells you nothing. `size_t bytes_read;`
does. The same applies to types: `struct user_store` is better than
`struct us`. Rename aggressively once tests cover the code.

### Duplication

If the same logic appears twice, consider extracting it. But beware premature
abstraction: two similar-looking pieces of code that change for different
reasons are better left separate. The rule is "rule of three" — extract when
you see it a third time and understand why it repeats.

### Long functions

A function that needs a diagram to read should be split. In C, long functions
are also harder to reason about for ownership: the more exits, the more
`free`s to get right. Small functions, each with a clear ownership contract
("this returns memory you must free", or "this borrows your pointer"), are
much easier to keep correct.

### Magic values

Give numbers and strings names:

```c
#define MAX_CONNECTIONS 16
const int seconds_per_minute = 60;
```

The name documents the intent and gives you one place to change the value.

### Deep nesting

Replace nested `if`s with guard clauses and early returns:

```c
if (amount < 0) {
    return WALLET_ERR_NEGATIVE_AMOUNT;
}
if (amount > w->balance) {
    return WALLET_ERR_INSUFFICIENT_FUNDS;
}
/* happy path is now flat */
```

### Data clumps and primitive obsession

If the same three parameters always travel together — `(char *buf, size_t len,
size_t pos)` — they want to be a struct. If a bare `int` represents something
with rules, consider a type that carries the rules.

### C-specific checks

- **`const`-correctness.** If a function does not modify what a pointer points
  at, say so. `const` turns a class of bug into a compiler error, and it
  documents the contract at the call site.
- **Ownership.** Can you say, for every pointer, who frees it and when? If not,
  the code needs restructuring before it needs testing.
- **Indirection.** Repeated `***` is a smell. Often a struct or an extra local
  variable flattens it.
- **`static`.** Helpers that are not part of the header should be `static`, so
  the linker and the reader both know they are private.
- **Headers.** Does each header include only what it needs? Does it include
  `<stddef.h>` for `size_t`? A clean header is a clean interface.
- **Error paths.** Re-read every failure branch. That is where the leaks are.
  Run the sanitizers and trust them.

## Your tooling does the boring parts

- `cmake --build build --target format` keeps style out of the conversation.
- Rename and extract-function in your editor, then re-run the tests.
- The compiler with `-Wall -Wextra -Werror` catches a surprising amount of
  structural sloppiness before you even run anything.

## Wrapping up

- Refactoring changes structure, not behaviour; tests make it safe
- Green first, small steps, one change at a time
- Comments-that-explain-what, long functions, duplication, magic values and
  nesting are the usual smells
- In C, add: `const`-correctness, ownership clarity, and clean error paths
- Let the compiler, the formatter and the sanitizers do the mechanical work

Refactoring is not a phase at the end. It is the punctuation between TDD
cycles — the "Refactor" step you have been running after every green test in
this book, taken as seriously as writing the code in the first place.
