#ifndef EXEC_H
#define EXEC_H

#include <stddef.h>

/*
 * Run a program and capture its standard output. argv is NULL-terminated and
 * argv[0] is the program name (looked up on PATH). No shell is involved, so
 * arguments are passed through untouched.
 *
 * Returns the number of bytes captured, or -1 if the pipe or fork failed.
 * The output is NUL-terminated and truncated to cap - 1 bytes.
 */
int exec_run(char *const argv[], char *out, size_t cap);

#endif /* EXEC_H */
