# Error types

**[You can find all the code for this chapter here](error-types/)**

A reader asked how to give callers useful information when something fails. Go
has an `error` interface you can flesh out with a custom type. C has no
exceptions and no interface — but a struct with a code and a message is
surprisingly close, and it maps straight onto an HTTP API.

## Errors are values

The [pointers and errors](pointers-and-errors.md) chapter used a bare `enum`.
That is enough to branch on, but not to explain. Let us carry a message too:

```c
struct app_error {
    int code;
    char message[128];
};
```

`code` is for the program (branch on it, map it to an HTTP status); `message`
is for the human (log it, show it). Filling it in is a printf-style call:

```c
void error_set(struct app_error *err, int code, const char *fmt, ...)
{
    va_list args;

    err->code = code;

    va_start(args, fmt);
    vsnprintf(err->message, sizeof err->message, fmt, args);
    va_end(args);
}
```

`va_list`, `va_start` and `vsnprintf` are how you write your own `printf`-like
function. The `__attribute__((format(printf, 3, 4)))` on the declaration asks
the compiler to type-check the format string against the arguments, so a wrong
`%d` is a compile error, not a crash.

## Write the test first

A service that registers users, using the error struct:

```c
static void test_empty_name_is_invalid(void)
{
    struct users users;
    struct app_error err;

    users_init(&users);

    CHECK_TRUE(!users_register(&users, "", &err));
    CHECK_INT(err.code, APP_ERR_INVALID);
    CHECK_STR(err.message, "name must not be empty");
}
```

Notice we assert on **both** the code and the message. The code is the
contract; the message is what a caller will actually read in a log. Testing the
message catches the bug where you set the right code and the wrong text — or
forget to set the text at all.

## Write enough code to make it pass

```c
bool users_register(struct users *users, const char *name, struct app_error *err)
{
    error_clear(err);

    if (name[0] == '\0') {
        error_set(err, APP_ERR_INVALID, "name must not be empty");
        return false;
    }
    if (contains(users, name)) {
        error_set(err, APP_ERR_CONFLICT, "user %s already exists", name);
        return false;
    }
    /* ... */
    return true;
}
```

The pattern has three rules, and they are worth following everywhere:

1. **Clear the error first.** A caller should be able to reuse one
   `struct app_error` across calls; a stale error from the last call is a bug.
2. **Return a boolean for success.** `false` means "look at `err`". The error
   is an *out*-parameter, like `int *out` in the [maps](maps.md) chapter.
3. **Set exactly one error**, with context. `"user chris already exists"` is
   worth ten `"conflict"`s.

## From codes to HTTP

The payoff is a single mapping, in one place:

```c
int error_http_status(int code)
{
    switch (code) {
    case APP_OK:
        return 200;
    case APP_ERR_INVALID:
        return 400;
    case APP_ERR_NOT_FOUND:
        return 404;
    case APP_ERR_CONFLICT:
        return 409;
    case APP_ERR_INTERNAL:
        return 500;
    default:
        return 500;
    }
}
```

Test it exhaustively, including the default — because "unknown code" is a
branch you will hit one day, and you want it to be a deliberate `500`, not
whatever the stack happened to hold.

```c
CHECK_INT(error_http_status(9999), 500);
```

And an error becomes a response, with the right status and a JSON body:

```c
void error_write_response(const struct app_error *err,
                          struct http_response *response)
{
    char body[256];

    snprintf(body, sizeof body, "{\"error\":\"%s\"}", err->message);
    http_response_json(response, error_http_status(err->code), body);
}
```

> The message is interpolated into JSON here without escaping. For a message
> built only from our own `error_set` calls that is fine; for a message that
> includes user input, escape it the way the [json](json.md) chapter's encoder
> does. `{"error":"bad \"quote"}` is not valid JSON. Test messages that contain
> a quote, and you will find this immediately.

## Errors versus exceptions

In Go, a function advertises failure in its return type (`(int, error)`), and
you unwrap a custom error with `errors.As`. In C, there are no exceptions at
all. You have three options:

- a bare return code for simple functions (`enum wallet_err`)
- an out-parameter error for context (this chapter)
- `errno`, set by libc, for system calls

The out-parameter style is the most useful for application code, because it
composes: a function can return a value *and* an error, and the error can carry
as much context as you like.

One thing Go has that C does not: the compiler checks that you *considered* the
error. In C, forgetting to check a return value is silent. `-Wall` will warn
you about some ignored results (`[[nodiscard]]` helps), but the discipline is
yours. The habit that makes it manageable is the same as in the
[refactoring checklist](refactoring-checklist.md): every function that can
fail takes an error, and every call site checks it.

## Wrapping up

What we have covered:

- A struct carrying a code and a message — errors as values
- Writing a variadic function with `va_list` and `vsnprintf`
- `__attribute__((format(...)))` for compile-time format checking
- Clear-then-set, return boolean, set one contextual error
- Mapping codes to HTTP statuses, with a tested default
- Why C makes you remember to check, and how to make that a habit

### Additional material

- [`errno` and error handling](https://en.cppreference.com/w/c/error)
- [`stdarg` and variadic functions](https://en.cppreference.com/w/c/variadic)
