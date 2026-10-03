# Pointers & errors

**[You can find all the code for this chapter here](pointers-and-errors/)**

We are going to build a `Wallet` that you can pay into and out of. Along the
way we meet the two things every C programmer has to internalise: mutating a
value through a pointer, and reporting failure without exceptions.

## Write the test first

```c
#include "mctest.h"
#include "wallet.h"

static void test_deposit(void)
{
    struct wallet w = {.balance = 0};

    wallet_deposit(&w, 10);
    CHECK_INT(wallet_balance(&w), 10);

    wallet_deposit(&w, 5);
    CHECK_INT(wallet_balance(&w), 15);
}
```

Notice the `&w`. We pass the *address* of the wallet, because `wallet_deposit`
needs to change it. This is the C equivalent of a Go pointer receiver.

## Try to run the test

```text
pointers-and-errors/v1/wallet_test.c:8:5: error: implicit declaration of function ‘wallet_deposit’ [-Werror=implicit-function-declaration]
```

The header:

```c
struct wallet {
    int balance;
};

void wallet_deposit(struct wallet *w, int amount);
int wallet_balance(const struct wallet *w);
```

The job of a `const` pointer is to promise the caller "this function will not
write through me". `wallet_balance` takes `const struct wallet *`, so if you
ever accidentally assign to `w->balance` in there, the compiler stops you.

## Write enough code to make it pass

```c
void wallet_deposit(struct wallet *w, int amount)
{
    w->balance += amount;
}

int wallet_balance(const struct wallet *w)
{
    return w->balance;
}
```

`w->balance` is sugar for `(*w).balance`: dereference the pointer, then access
the field.

### Why not pass the wallet by value?

If `wallet_deposit` took `struct wallet w` instead, it would get a *copy*.
Adding to `w.balance` would change the copy, and the caller's wallet would be
untouched. When a function's whole job is to mutate something, it must be
given the address. When your instinct is "this should work but nothing
changes", nine times out of ten you forgot a `&`.

## More requirements: withdrawing, and failing

Now let's withdraw money. The problem is that a withdrawal can fail, and a
`void` return gives us nowhere to report that.

Go would return `(error)` and you would check `if err != nil`. C has no
`error` interface and no `nil`. What it has is **return codes**, and this is
the single biggest culture shock coming from Go.

There is a spectrum of C error handling:

- return a special value (`-1`, `NULL`) — fast, but easy to ignore and
  ambiguous
- return an `enum` of error codes — what we will do; explicit and checkable
- set the global `errno` and return `-1` — used by libc, awkward for anything
  larger
- longjmp/setjmp — C's "exceptions", and best avoided

For a domain type like ours, an `enum` is clear, language-checkable, and
impossible to mistake for a valid result.

## Write the test first

```c
static void test_withdraw(void)
{
    struct wallet w = {.balance = 20};

    CHECK_INT(wallet_withdraw(&w, 5), WALLET_OK);
    CHECK_INT(wallet_balance(&w), 15);
}

static void test_withdraw_insufficient_funds_leaves_balance_alone(void)
{
    struct wallet w = {.balance = 20};

    CHECK_INT(wallet_withdraw(&w, 100), WALLET_ERR_INSUFFICIENT_FUNDS);
    CHECK_INT(wallet_balance(&w), 20);
}
```

The second test is the interesting one: not only does it check the error, it
checks that a failed withdrawal did **not** change the balance. A test that
only checked the return code would pass even if we had already subtracted the
money. State invariants are worth asserting explicitly.

## Try to run the test

```text
pointers-and-errors/v2/wallet_test.c:5:52: error: implicit declaration of function ‘wallet_withdraw’ [-Werror=implicit-function-declaration]
```

Add to the header:

```c
enum wallet_err {
    WALLET_OK = 0,
    WALLET_ERR_INSUFFICIENT_FUNDS,
    WALLET_ERR_NEGATIVE_AMOUNT,
};

enum wallet_err wallet_withdraw(struct wallet *w, int amount);
```

Two C conventions are doing quiet work here. `WALLET_OK = 0` means that "no
error" is *falsy*, so `if (!err)` reads naturally and `err` doubles as a
boolean. And prefixing every enumerator with `WALLET_` avoids the collisions
you would otherwise get, because C enum constants share the global namespace —
there is no `WALLET_ERR_INSUFFICIENT_FUNDS` scoping.

## Write the minimal amount of code for the test to run and check the failing test output

```c
enum wallet_err wallet_withdraw(struct wallet *w, int amount)
{
    (void)w;
    (void)amount;
    return WALLET_OK;
}
```

```text
    pointers-and-errors/v2/wallet_test.c:9: test_withdraw: got 20 want 15
    pointers-and-errors/v2/wallet_test.c:16: test_withdraw_insufficient_funds_leaves_balance_alone: got 0 want 1
--- FAIL test_withdraw
--- FAIL test_withdraw_insufficient_funds_leaves_balance_alone
```

Both failures are useful: the first says money was not taken out, the second
says we did not report the error. The enum values printed as numbers because
that is what they are.

## Write enough code to make it pass

Check the failure cases *before* changing any state, so that a failed
withdrawal is a no-op:

```c
enum wallet_err wallet_withdraw(struct wallet *w, int amount)
{
    if (amount < 0) {
        return WALLET_ERR_NEGATIVE_AMOUNT;
    }
    if (amount > w->balance) {
        return WALLET_ERR_INSUFFICIENT_FUNDS;
    }

    w->balance -= amount;
    return WALLET_OK;
}
```

Add a test for the negative case too:

```c
static void test_withdraw_negative(void)
{
    struct wallet w = {.balance = 20};

    CHECK_INT(wallet_withdraw(&w, -5), WALLET_ERR_NEGATIVE_AMOUNT);
    CHECK_INT(wallet_balance(&w), 20);
}
```

Run the tests. Green.

## Refactor: names for the errors

An error code is not much use to a user if you print `1`. Give the errors
names, and test the names:

```c
const char *wallet_err_string(enum wallet_err err)
{
    switch (err) {
    case WALLET_OK:
        return "ok";
    case WALLET_ERR_INSUFFICIENT_FUNDS:
        return "insufficient funds";
    case WALLET_ERR_NEGATIVE_AMOUNT:
        return "negative amount";
    }
    return "unknown error";
}
```

`switch` over an enum is idiomatic C, and `-Wall` will warn you if you forget
a case. The final `return` is not dead code to the compiler, so it must be
there — but if you add a new error and forget to handle it, the warning will
tell you.

### Choose error values with an eye to the caller

Notice we did *not* use an out-parameter for the error. A tempting design is:

```c
int wallet_withdraw(struct wallet *w, int amount, enum wallet_err *err);
```

That is a common C shape, and sometimes necessary when the function also has a
value to return. For a function whose only output is "did it work", returning
the error *is* the output. Fewer parameters, fewer ways to get it wrong.

## Wrapping up

We have covered:

- Passing a struct by pointer so a function can mutate it
- `const T *` as a promise not to modify the pointee
- Looking up struct fields through a pointer with `->`
- Error handling with an `enum` return code: check before you mutate
- `enum` constants live in one big namespace, so prefix them
- Tests that assert invariants (the balance is unchanged) and not just the
  return value

Coming from Go you will miss the `error` interface for a while. What you gain
is that *every* possible failure is visible in the type of the function: if
`wallet_withdraw` returns `enum wallet_err`, you cannot pretend it doesn't
fail.

### Additional material

- [`errno` and error handling in C](https://en.cppreference.com/w/c/error)
- [My favourite way to avoid C error-handling boilerplate](https://www.open-std.org/jtc1/sc22/wg14/www/docs/n2289.pdf) — a standards proposal, for context
