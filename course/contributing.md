# Contributing

This book is a work in progress, and contributions are welcome.

## What makes a good chapter

A chapter is a tutorial, not a reference. It follows the TDD loop:

- Start with a concrete requirement, before any code
- Write a failing test, and show the failure
- Make the compiler happy, then make the test pass
- Refactor, then repeat with the next requirement
- Finish with `## Wrapping up`

Read `CLAUDE.md` for the full writing guide, and `source/` for the Go book
this one is modelled on.

## Code rules

- Target **C23** and compile with `-Wall -Wextra -Werror`
- One directory per chapter, one sub-directory per version (`v1`, `v2`, ...)
- Every version must be a self-contained CMake target
- Use the shared `mctest` harness; do not add another test framework
- Prefer the standard library and POSIX APIs; vendor nothing
- Format with `cmake --build build --target format`

## Before you open a pull request

```sh
./build.sh
cmake --build build --target check-format
```

and, for anything to do with memory or threads:

```sh
cmake -S . -B build-asan -DENABLE_SANITIZERS=ON
cmake --build build-asan
ctest --test-dir build-asan --output-on-failure

cmake -S . -B build-tsan -DENABLE_TSAN=ON
cmake --build build-tsan
ctest --test-dir build-tsan --output-on-failure
```

All compiler warnings are errors, so the build itself is most of the review.

## Reporting problems

If a chapter does not compile, a test is wrong, or an explanation is unclear,
please raise an issue with the chapter name and the exact command you ran.
Reproducing a command is far more useful than describing the symptom.
