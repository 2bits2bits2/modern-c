# Modern C with Tests — writing guide

This book is the C sibling of Chris James's "Learn Go with Tests"
(`../source`). The goal is not to clone the Go text, but to reproduce its
*teaching method* and *voice* for modern C. When drafting or editing a chapter,
read an actual chapter from `../source` first rather than guessing.

## Voice, in one paragraph

Casual, collaborative, "we're solving this together" — heavy on "we"/"let's",
second person "you" for instructions. Contractions everywhere. Short-to-medium
sentences; paragraphs before a code block are almost always 1–3 sentences.
Dry, understated humour lands as single-word or single-line beats ("Yuck.",
"Ouch.", "Perfect!", "As expected") rather than running jokes — it's seasoning,
not the dish. Opinions are stated with real conviction but usually hedged with
"I feel", "in my experience", or an immediate "but/however" qualifier;
alternatives are acknowledged rather than dismissed.

## Structure

- Tutorial chapters open with a concrete scenario (a product owner, a
  colleague, a Slack/Reddit question) *before* any code, and the first line is
  almost always a bolded link to the code for that chapter.
- New concepts are motivated by a failing test or a compiler error wherever
  possible, not front-loaded as "here is concept X" — except when introducing
  an entire new library's mental model (pthreads, poll, atomics), which gets a
  short upfront "just enough information" section.
- The TDD ritual headings recur near-verbatim:
  `## Write the test first`, `## Try to run the test`,
  `## Write the minimal amount of code for the test to run and check the failing test output`,
  `## Write enough code to make it pass`, `## Refactor`.
  Steps can be merged when one is trivial, but don't rename them.
- Chapters close with `## Wrapping up`, usually a bulleted "what we've
  covered", sometimes split into named sub-lists. An `### Additional material`
  section with links is common at the very end.
- Failures (compiler errors, sanitizer reports, segfaults) are shown verbatim,
  then narrated calmly afterwards. Reproduce real output by actually running
  the command; never hand-write plausible-looking error text.
- Blockquotes (`>`) are reserved for quoting external sources, never for the
  author's own voice.

## C-specific housekeeping

- Target **C23** and compile with `-std=c23`/`-std=c2x`, `-Wall -Wextra -Werror`.
  GCC 13 accepts `-std=c2x`; CMake's `C_STANDARD 23` handles the mapping.
- Every chapter version directory is an independent CMake target, so earlier
  chapters keep compiling as later ones evolve the same domain differently.
- The shared test harness lives in `test/` (`mctest.h` + `mctest.c`). Chapters
  link the `mctest` library. Do not invent a different harness per chapter.
- Prefer the standard library and POSIX APIs over third-party dependencies.
  This book vendors nothing.
- Show `CHECK_*` assertions, not bare `assert()`, in chapter code.
- Before treating code as finished: configure with CMake, build, run
  `ctest --output-on-failure`, and for concurrency chapters configure with
  `-DENABLE_SANITIZERS=ON`. Compiler warnings are errors in this project.
- Never hand-write compiler errors or sanitizer traces in prose — reproduce
  them for real.

## Scope

The book mirrors the Go book's table of contents, substituting C-native topics
where a Go feature has no direct equivalent (e.g. goroutines → pthreads,
reflection → `_Generic`/`typeof`, HTML templates → string templating).
