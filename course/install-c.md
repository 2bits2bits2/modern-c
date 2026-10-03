# Install a C toolchain, set up environment for productivity

C is a small language but it comes with a big assumption: you bring your own
compiler, build system and testing tools. Go hands you all of that in one
binary; in C we assemble it. This chapter does that once, so every later
chapter can just be about the code.

## A compiler

You need a C23 compiler. Any recent one will do:

- [GCC](https://gcc.gnu.org/) 13 or later
- [Clang](https://clang.llvm.org/) 18 or later

On Debian and Ubuntu based systems:

```sh
sudo apt install build-essential
```

On macOS, the compiler ships with the Xcode command line tools:

```sh
xcode-select --install
```

Check you have something:

```sh
cc --version
```

In this book you will see `cc` rather than `gcc` or `clang`. `cc` is the
conventional name for "the system C compiler", and on almost every machine it
is a link to one of the two.

### Which standard?

The C language has a slow, careful standards process. The revisions you are
likely to meet are:

- **C99** — the one that finally looked like a modern language for its time
- **C11** — threads, atomics and `_Generic`
- **C17** — a bug-fix release, mostly
- **C23** — the version we use in this book

C23 gives us `nullptr`, `constexpr`, `typeof`, `auto` type inference, binary
literals, digit separators, `[[attributes]]` and much more. It makes C feel
noticeably less like a 1970s language.

GCC 13 spells the flag `-std=c2x`; GCC 14 and Clang 18 accept `-std=c23`.
CMake, which we use below, hides this difference for us.

### Turn the warnings up

C compilers are permissive by default for historical reasons. We will be
strict, because the compiler is one of our best teachers:

```sh
cc -std=c2x -Wall -Wextra -Werror
```

- `-Wall -Wextra` turn on a large set of useful warnings
- `-Werror` makes warnings into errors, so you cannot ignore them during the
  build

This matters even more in C than in Go. Whole classes of bugs that other
languages catch for you — using an uninitialised variable, forgetting a
`return`, a missing function declaration — are only warnings here until you
ask for them to be errors.

> **Listen to the compiler.** Throughout this book, when the compiler
> complains, read it. It is almost always telling you what to do next. The
> TDD loop in C has an extra step early on: make the *compiler* happy, then
> make the *test* pass.

## CMake

Compiling `hello.c` by hand gets old fast once you have a test file, a
library, and dozens of chapters. We use [CMake](https://cmake.org/), the
de-facto standard build system for C projects.

Install it from [cmake.org/download](https://cmake.org/download/) or your
package manager:

```sh
sudo apt install cmake
```

You need version 3.16 or newer:

```sh
cmake --version
```

You will not have to write much CMake by hand; the book's repository already
has a top-level `CMakeLists.txt`, and each chapter adds a line or two.

## The build and test loop

From the `course` directory:

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

- `cmake -S . -B build` *configures* the project once (you only re-run it when
  you add files)
- `cmake --build build` *compiles* everything that changed
- `ctest` *runs* the tests and stops to show you the output of any failures

That is our equivalent of `go test`.

## Sanitizers

One of the best inventions since C was standardised is the sanitizers. They
instrument your program so it can tell you about mistakes you would otherwise
find the hard way:

- **AddressSanitizer (ASan)** catches out-of-bounds access, use-after-free and
  leaks
- **UndefinedBehaviorSanitizer (UBSan)** catches signed overflow, misaligned
  pointers, bad shifts and more

The book supports a sanitizer build out of the box:

```sh
cmake -S . -B build -DENABLE_SANITIZERS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

Get into the habit of running the sanitizer build, especially for the memory
and concurrency chapters. A bug that ASan finds in one second can otherwise
cost you an afternoon with a debugger.

## A debugger

You will need a debugger eventually. Two good ones:

- [gdb](https://sourceware.org/gdb/)
- [lldb](https://lldb.llvm.org/)

We will not teach a whole debugger here, but you should know how to get a
stack trace from a crash. Compile with `-g` (CMake's `Debug` build type does
this) and then:

```sh
gdb ./build/hello-world/hello_v1_program
(gdb) run
(gdb) bt
```

`bt` is "backtrace": it shows the chain of function calls that led to the
crash. That single command solves a surprising number of problems.

## Formatting

Go has `gofmt`, and nobody argues about Go style. C has no such thing: there is
no single formatter that everyone uses. The closest thing to a standard is
[clang-format](https://clang.llvm.org/docs/ClangFormat.html), which is what
this book uses.

Because clang-format is configurable, "the same output everywhere" only holds
if you commit the configuration. The course ships a `.clang-format`, so your
editor and your teammates all agree. Format everything in one command:

```sh
cmake --build build --target format
```

Or check that nothing is unformatted, the way CI would:

```sh
cmake --build build --target check-format
```

If you want to format only the lines you have touched, `git clang-format`
does exactly that:

```sh
git clang-format
```

A formatter is not about being right — it is about never having the argument,
and never reviewing a diff that is mostly whitespace.

### Additional material

- [clang-format style options](https://clang.llvm.org/docs/ClangFormatStyleOptions.html)
- [EditorConfig](https://editorconfig.org/) — the course also ships an
  `.editorconfig` for indentation and line endings

## Refactoring and your tooling

A big emphasis of this book is refactoring. Your editor can help:

- **Rename** a symbol across files
- **Extract function** from a selection
- **Format on save** with [clang-format](https://clang.llvm.org/docs/ClangFormat.html)
- **Run the tests** without leaving the editor

Two more habits that pay off in C specifically:

- **Compile often.** A C compiler checks one translation unit at a time, so
  errors surface earlier if you build continuously.
- **Read the declaration, not the definition.** In C you will spend most of
  your time in header files. Getting comfortable reading a `.h` is a skill in
  itself.

## Wrapping up

At this point you should have:

- A C23 compiler, `cc`
- CMake 3.16+
- A build and test command you can run from the `course` directory
- A way to turn on sanitizers
- A debugger

C does not give you much for free, but what it does give you is small enough
to understand completely. That is the trade, and it is a good one. On to
Hello, World.
