# Modern C with Tests

Learn modern C (C23) by writing tests.

This book is a companion to [Learn Go with Tests](https://github.com/quii/learn-go-with-tests),
re-imagined for C. Instead of a language with testing built in, we build a
tiny test harness together in the first chapter and then use it to explore the
language, one failing test at a time.

## Why

* Explore modern C (C23) by writing tests
* **Get a grounding with TDD.** C is a small language, so a lot of the design
  work happens in your head and in your build. Tests give you the safety net to
  refactor without fear.
* Be confident that you can write robust, well-tested systems in C
* Learn the tooling that makes C pleasant in 2026: CMake, sanitizers, static
  analysis and a debugger

## Who this is for

* People who are curious about C, or who last wrote it before C99
* People who already know some C but want to practise TDD
* People who want to understand the machinery under higher-level languages

## What you'll need

* A computer running Linux, macOS or WSL
* A C23 compiler (GCC 13+ or Clang 18+)
* [CMake](https://cmake.org/) 3.16+
* A text editor
* Some programming experience (`if`, variables, functions, the terminal)

## How the code is organised

Every chapter has a directory with numbered sub-directories (`v1`, `v2`, ...).
Each version is a self-contained snapshot of the code at that point in the
chapter, exactly as it appeared after the latest TDD cycle. Later chapters
never change earlier ones, so old code keeps compiling.

```
course/
  CMakeLists.txt      # builds every chapter and registers its tests
  test/               # our tiny test harness (mctest) + process helpers
  app/                # the growing application (Build an application)
  hello-world/
    v1/
    v2/
    ...
  integers/
    v1/
    ...
```

The `app/` library is shared by the Build an application chapters: each one
adds files to it rather than rewriting them, so earlier chapters keep
compiling. The fundamentals and testing chapters are self-contained, with
numbered snapshots as above.

## Running the tests

From the `course` directory:

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

To build with the address and undefined-behaviour sanitizers:

```sh
cmake -S . -B build -DENABLE_SANITIZERS=ON
```

## Formats

This book is written in Markdown and can be rendered with
[mdBook](https://rust-lang.github.io/mdBook/), GitBook or plain Markdown
viewers.

## Building and publishing the book

The GitBook-style site is generated with mdBook:

```sh
cd course
mdbook build      # writes the site to course/book/
mdbook serve      # preview it locally at http://localhost:3000
```

The [`Deploy book to GitHub Pages`](../.github/workflows/mdbook.yml) workflow
builds the book on every push to `main` and publishes it to
<https://2bits2bits2.github.io/modern-c/>. For this to work, set the repository's
**Settings → Pages → Build and deployment → Source** to **GitHub Actions**.

## Table of contents

See [SUMMARY.md](SUMMARY.md).

## Feedback

Issues and pull requests are welcome.

[MIT license](LICENSE.md)
