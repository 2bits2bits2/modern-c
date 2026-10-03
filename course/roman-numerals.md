# Intro to property based tests

**[You can find all the code for this chapter here](roman-numerals/)**

Time for a kata, and a new testing technique. We are going to convert numbers
to [Roman numerals](https://en.wikipedia.org/wiki/Roman_numerals) and back
again. The interesting part is not the algorithm; it is how we test it. A
handful of example tests will pass even if the code is wrong for hundreds of
inputs. A **property** test checks a rule that must hold for *every* input.

## Write the test first

Start with a few known cases, the way you always would:

```c
static void test_known_numerals(void)
{
    char buf[32];

    to_roman(1, buf, sizeof buf);
    CHECK_STR(buf, "I");
    to_roman(4, buf, sizeof buf);
    CHECK_STR(buf, "IV");
    to_roman(9, buf, sizeof buf);
    CHECK_STR(buf, "IX");
    to_roman(1984, buf, sizeof buf);
    CHECK_STR(buf, "MCMLXXXIV");
    to_roman(3999, buf, sizeof buf);
    CHECK_STR(buf, "MMMCMXCIX");
}
```

## Write enough code to make it pass

The greedy algorithm is all you need: walk a table of values from largest to
smallest, emitting the symbol each time you can.

```c
static const struct {
    int value;
    const char *symbol;
} symbols[] = {
    {1000, "M"}, {900, "CM"}, {500, "D"}, {400, "CD"}, {100, "C"},
    {90, "XC"},  {50, "L"},   {40, "XL"}, {10, "X"},   {9, "IX"},
    {5, "V"},    {4, "IV"},   {1, "I"},
};

int to_roman(int n, char *buf, size_t cap)
{
    if (n < 1 || n > 3999) {
        return -1;
    }

    size_t pos = 0;
    for (size_t i = 0; i < sizeof symbols / sizeof symbols[0]; i++) {
        while (n >= symbols[i].value) {
            size_t len = strlen(symbols[i].symbol);
            if (pos + len + 1 > cap) {
                return -1;
            }
            memcpy(buf + pos, symbols[i].symbol, len);
            pos += len;
            n -= symbols[i].value;
        }
    }

    buf[pos] = '\0';
    return (int)pos;
}
```

Notice that the table includes the two-symbol cases (`CM`, `XC`, ...) rather
than special-casing them. Putting the data in a table turns a pile of `if`s
into a loop. This is a recurring C theme: data beats code.

## The partner function, and a property

Now the part that makes this chapter worth its salt. Write `from_roman`, which
parses a numeral back to a number. Then assert a property:

> For every `n` in `1..3999`, `from_roman(to_roman(n)) == n`.

```c
static void test_round_trip_property(void)
{
    char buf[32];

    for (int n = 1; n <= 3999; n++) {
        CHECK_INT(to_roman(n, buf, sizeof buf) > 0, 1);
        CHECK_INT(from_roman(buf), n);
    }
}
```

One test, thousands of inputs. If `to_roman` mishandles 47, or `from_roman`
misreads `XLIX`, this test fails and tells you the number. Compare that to
picking a few examples and hoping you picked the hard ones.

The parsing algorithm is a neat trick: scan right to left, and if a symbol is
smaller than the one to its right, subtract it instead of adding. That handles
subtractive notation like `IV` without any lookahead.

```c
int from_roman(const char *s)
{
    int total = 0;
    int prev = 0;
    size_t len = strlen(s);

    if (len == 0) {
        return -1;
    }

    for (size_t i = len; i-- > 0;) {
        int value = /* value of s[i], or -1 */;
        if (value < 0) {
            return -1;
        }
        if (value < prev) {
            total -= value;
        } else {
            total += value;
            prev = value;
        }
    }

    return total;
}
```

The loop `for (size_t i = len; i-- > 0;)` is an idiom for counting down through
an unsigned index without underflowing: it tests the *old* value of `i`, then
decrements, and stops before `i` would wrap around.

## Going further: random inputs and shrinking

A `for` loop over the whole range is the best kind of property test when the
range is small. When it is large or infinite, you sample randomly. Two things
make random property testing powerful:

- **Random generation** of inputs, so you explore corners you would not have
  thought of
- **Shrinking**: when a random input fails, automatically search for a smaller
  input that still fails, so the reported bug is minimal and legible

C has no property-testing framework in the standard library. You can write a
simple generator with `rand()`:

```c
for (int trial = 0; trial < 1000; trial++) {
    int n = 1 + rand() % 3999;
    /* ... same round trip ... */
}
```

Shrinking is more work, and is the reason people reach for tools. For most C
code, an exhaustive loop over the meaningful range is simpler and stronger.
Reach for randomness when you cannot enumerate.

## Wrapping up

What we have covered:

- The Roman numerals kata, solved with a data table and a greedy loop
- Parsing subtractive notation by scanning right to left
- **Property based testing**: asserting a rule over all inputs, not examples
- `for (size_t i = len; i-- > 0;)` as a safe reverse loop
- Randomised inputs, and why shrinking matters

This is one of the highest-leverage ideas in testing. The Go book reaches for
`testing/quick`; C does not give us one, but the *technique* is language
independent and, for a bounded domain like this, it is just a `for` loop.

### Additional material

- [Property-based testing](https://en.wikipedia.org/wiki/Property_testing)
- [QuickCheck](https://hackage.haskell.org/package/QuickCheck) — where the idea comes from
