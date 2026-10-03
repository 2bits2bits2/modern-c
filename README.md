# Modern C with Tests

This repository contains two things:

- **`course/`** — *Modern C with Tests*, a test-driven introduction to modern C
  (C23), built with the same teaching method as the Go book below.
- **`source/`** — a copy of Chris James's
  [Learn Go with Tests](https://github.com/quii/learn-go-with-tests), kept as
  the reference and inspiration for the C course.

Start with [`course/README.md`](course/README.md), or dive into the table of
contents at [`course/SUMMARY.md`](course/SUMMARY.md).

## The C course at a glance

- Learn C23 by writing tests, one failing test at a time
- Build your own tiny test harness in Chapter 1
- CMake build, AddressSanitizer/UndefinedBehaviorSanitizer and
  ThreadSanitizer support
- Every chapter's code is real, compiles warning-free, and is exercised by
  CTest

```sh
cd course
./build.sh
```

## License

Both projects are MIT licensed. See [`course/LICENSE.md`](course/LICENSE.md)
and [`source/LICENSE.md`](source/LICENSE.md).
