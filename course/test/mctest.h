#ifndef MCTEST_H
#define MCTEST_H

/*
 * mctest — a tiny test harness for Modern C with Tests.
 *
 * It is deliberately small. The whole implementation lives in mctest.c and
 * you will build it up yourself in the Hello, World chapter.
 */

#include <stddef.h>

/* Run a named test function. */
void run_test(const char *name, void (*fn)(void));

/* Assertions. Each one counts as a check; failing ones are reported. */
void check_str(const char *got, const char *want, const char *file, int line);
void check_mem(const void *got, const void *want, size_t n, const char *file,
               int line);
void check_int(long long got, long long want, const char *file, int line);
void check_double(double got, double want, double tolerance, const char *file,
                  int line);
void check_true(int condition, const char *expr, const char *file, int line);
void check_fail(const char *file, int line, const char *fmt, ...)
    __attribute__((format(printf, 3, 4)));

/* Print a summary and return 0 if every check passed. */
int test_summary(void);

/* The file and line the assertion was written on. */
#define HERE __FILE__, __LINE__

#define RUN_TEST(fn) run_test(#fn, fn)

#define CHECK_STR(got, want) check_str((got), (want), HERE)
#define CHECK_MEM(got, want, n) check_mem((got), (want), (n), HERE)
#define CHECK_INT(got, want) check_int((got), (want), HERE)
#define CHECK_DOUBLE(got, want, tol) check_double((got), (want), (tol), HERE)
#define CHECK_TRUE(cond) check_true((cond), #cond, HERE)
#define FAIL(...) check_fail(HERE, __VA_ARGS__)

#endif /* MCTEST_H */
