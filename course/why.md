# Why unit tests and how to make them work for you

C is a language that will not save you from yourself. It has no garbage
collector, no bounds checks, no exceptions, and undefined behaviour lurking in
the corners. That sounds like a reason to be careful, and it is — but "be
careful" is not a technique. Tests are.

A test is a second, executable description of what your code is supposed to do.
The compiler checks that your program is well-formed; the test checks that it is
*correct*. In a language where a single missing `free` can crash a program an
hour later, that second description is not a luxury.

## What good tests buy you in C specifically

- **A safety net for refactoring.** You will reorganise memory ownership many
  times. Without tests, every move of a `free` is a gamble.
- **A place to pin down edge cases.** Empty strings, `NULL`, zero length, the
  overflow boundary, the last byte of a buffer. These are where C bugs live,
  and a test is a place to write them down once.
- **A way to run your code under the sanitizers.** A test harness lets you
  exercise paths — error paths especially — under AddressSanitizer and
  ThreadSanitizer. Untested error paths are where leaks hide.
- **Documentation that cannot go stale.** A comment can lie. A passing test
  cannot.

## How to make them work for you

- **Fast.** If the suite takes a minute, you will stop running it. That is why
  we inject the clock in the [mocking](mocking.md) chapter instead of sleeping.
- **Focused.** One behaviour per test, with a name that describes the
  requirement. When it fails, the name should tell you what broke.
- **First.** Write the test before the code, and *watch it fail for the right
  reason*. A test you have never seen fail is a test you cannot trust.
- **Deterministic.** No real time, no network, no randomness you cannot
  control. Flaky tests train people to ignore failures.

## The TDD cycle in C

1. Write a test.
2. Make the **compiler** happy — declarations, types, signatures.
3. Run the test and read the failure. Is it the failure you expected?
4. Write enough code to pass.
5. Refactor, with the test as your net.
6. Repeat.

C adds step 2 to the familiar Go cycle, and it is not a nuisance — it is the
compiler teaching you the shape of your own program. "Listen to the compiler"
is a real workflow, not a slogan.

## Watch the talk

This chapter is a short adaptation of the introduction to
[Learn Go with Tests](https://github.com/quii/learn-go-with-tests). If you
prefer video, the reasoning behind it is in
[Ian Cooper's talk on TDD, where it all went wrong](https://www.youtube.com/watch?v=EZ05e7EMOLM) —
the title is tongue in cheek, and the advice is excellent.

## Wrapping up

- Tests are the safety net that makes C's manual memory management survivable
- Good tests are fast, focused, written first, and deterministic
- In C, the loop has an extra "satisfy the compiler" beat
- The sanitizers turn your test suite into a memory- and race-checking machine
