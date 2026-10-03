# Templating

**[You can find all the code for this chapter here](templating/)**

We need to take a string like `"Hello, {name}!"` and fill in the placeholders.
Go uses `text/template`; C has `printf`, which is powerful but positional —
you cannot easily name your substitutions. So we will build a small named
templating function, and along the way practise careful string handling.

## Write the test first

```c
static void test_replace_one(void)
{
    char buf[64];

    render("Hello, {name}!", "name", "Chris", buf, sizeof buf);

    CHECK_STR(buf, "Hello, Chris!");
}
```

## Write enough code to make it pass

The algorithm: walk the template, and whenever we see `{key}`, emit the value
instead.

```c
void render(const char *tmpl, const char *key, const char *value, char *buf,
            size_t cap)
{
    size_t pos = 0;
    size_t i = 0;
    size_t key_len = strlen(key);

    while (tmpl[i] != '\0' && pos + 1 < cap) {
        if (tmpl[i] == '{' && strncmp(tmpl + i + 1, key, key_len) == 0 &&
            tmpl[i + 1 + key_len] == '}') {
            for (size_t j = 0; value[j] != '\0' && pos + 1 < cap; j++) {
                buf[pos] = value[j];
                pos++;
            }
            i += key_len + 2;
        } else {
            buf[pos] = tmpl[i];
            pos++;
            i++;
        }
    }

    if (cap > 0) {
        buf[pos] = '\0';
    }
}
```

`strncmp` compares exactly `key_len` characters, which is what we want — we are
not comparing a whole string, just the slice between `{` and `}`. Then we check
that the character after the key really is `}`. Getting that check wrong would
make `{name}` match inside `{nameandaddress}`.

Every copy is guarded by `pos + 1 < cap`, so the buffer is never overflowed and
there is always room for the terminator.

Now the tests for the other cases:

```c
static void test_replace_every_occurrence(void)
{
    char buf[64];

    render("{x} and {x} again", "x", "42", buf, sizeof buf);

    CHECK_STR(buf, "42 and 42 again");
}

static void test_no_placeholder(void)
{
    char buf[64];

    render("no placeholders here", "x", "42", buf, sizeof buf);

    CHECK_STR(buf, "no placeholders here");
}
```

The "no placeholder" test matters: a function that mangles a template with
nothing to substitute is a classic bug, and it is easy to introduce by
dropping the trailing text.

## More requirements: several placeholders

One key is not enough. Real templates have many. Let us pass a table of
bindings:

```c
struct binding {
    const char *key;
    const char *value;
};

void render_all(const char *tmpl, const struct binding *bindings, size_t n,
                char *buf, size_t cap);
```

The change to the algorithm is small: when we find a `{...}`, look up the key
in the table rather than comparing it to one fixed key.

```c
if (tmpl[i] == '{') {
    const char *close = strchr(tmpl + i + 1, '}');
    const char *value =
        close != NULL
            ? lookup(bindings, n, tmpl + i + 1, (size_t)(close - (tmpl + i + 1)))
            : NULL;

    if (value != NULL) {
        /* copy value */
        i = (size_t)(close - tmpl) + 1;
        continue;
    }
}
/* otherwise copy the character unchanged */
```

`strchr` finds the closing brace for us, and `close - (tmpl + i + 1)` is pointer
arithmetic: the number of characters between two positions in the same array.
That length is then handed to `lookup`, which does the same `strncmp` idea over
the table.

The test with two bindings:

```c
static void test_multiple_bindings(void)
{
    char buf[64];
    const struct binding bindings[] = {
        {.key = "name", .value = "Chris"},
        {.key = "language", .value = "C"},
    };

    render_all("Hello, {name}! You are writing {language}.", bindings, 2, buf,
               sizeof buf);

    CHECK_STR(buf, "Hello, Chris! You are writing C.");
}
```

And a deliberate edge case: an unknown placeholder is left as-is, rather than
being silently deleted.

```c
render_all("{greeting}, {name}!", bindings, 1, buf, sizeof buf);
CHECK_STR(buf, "{greeting}, Chris!");
```

## Refactor

The single-pass version is fine, but there is a subtlety: because we copy the
output directly into `buf`, a `{key}` whose *value* contains a `{other}`
placeholder is not expanded further. That is almost always what you want — one
pass, no injection — but it is worth knowing that it is a choice.

If you kept going, the natural extensions are:

- Escaping a literal `{` (`{{` maybe), so users can output a brace
- A `binding` whose value is computed lazily, rather than a fixed string
- Writing to a `struct writer` (from the [dependency injection](dependency-injection.md)
  chapter) instead of a fixed buffer, so the output can be arbitrarily large

That last one is the most interesting: it removes the `cap` parameter and the
truncation concern entirely, because the writer grows as needed.

## Wrapping up

What we have covered:

- Scanning a string with an index, and copying byte by byte with bounds checks
- `strncmp` for partial comparisons, and why it beats `strcmp` here
- `strchr` and pointer subtraction (`close - start`) to measure a slice
- Handling many bindings with a table
- Deciding what to do with unknown placeholders
- Extending a fixed-buffer API to a `struct writer` when size is unbounded

C has no template engine in its standard library, which is a small loss and a
useful exercise. You now understand exactly what `printf` does not do for you,
and how to build the thing you actually wanted.

### Additional material

- [`strchr` and the string library](https://en.cppreference.com/w/c/string/byte)
- [printf format reference](https://en.cppreference.com/w/c/io/fprintf)
