# Anti-patterns

A short list of testing and TDD habits that cause more harm than good. None of
these are C-specific, but some bite harder here.

## Treating the test as a rubber stamp

If you write the test *after* the code and it passes immediately, you have
learned nothing. You do not know whether the test can fail, or whether it is
testing the thing you think. Write the test first, watch it fail, then make it
pass.

## Testing the implementation, not the behaviour

A test that asserts a helper was called exactly four times, or that an internal
buffer is exactly 64 bytes, will fail the next time you refactor even though
the program still works. Test the behaviour a caller can observe. In C the
temptation is strong because so much is observable — resist it.

## Reaching for a mocking framework

In C an interface is a struct of function pointers, so a test double is a small
struct. You almost never need a library, and a hand-written spy is easier to
read. See [mocking](mocking.md).

## `assert()` in place of a test harness

`assert` aborts on the first failure, gives no summary, and is disabled by
`NDEBUG` in release builds. It is a debugging aid, not a testing strategy. That
is why the book builds `mctest` in [Hello, World](hello-world.md).

## Ignoring the sanitizers

A green test run under no sanitizer is half a green test run. AddressSanitizer
finds leaks and out-of-bounds; ThreadSanitizer finds races. They are one
`-DENABLE_SANITIZERS=ON` away. Use them.

## Testing undefined behaviour

You cannot reliably test overflow, use-after-free, or out-of-bounds access by
observing the result — the compiler is allowed to do anything. Test the
*guard*: assert that `add_checked` returns `false` at the boundary, not that a
wrapped result "looks right".

## Slow, flaky tests

A test that sleeps for real seconds, or that depends on the network or the
clock, will eventually fail for reasons unrelated to your code. Inject the
dependency and control it. The [mocking](mocking.md) and
[dependency injection](dependency-injection.md) chapters are both about this.

## One giant test function

A test named `test_everything` that performs twenty checks is unreadable when
it fails. Split it. The harness runs every test and reports a summary, so
there is no cost to many small tests.
