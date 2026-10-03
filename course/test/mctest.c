#include "mctest.h"

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static int tests_run;
static int tests_failed;
static int checks_run;
static int checks_failed;
static const char *current_test = "<main>";

static void vreport(const char *file, int line, const char *fmt, va_list args)
{
    checks_failed++;
    printf("    %s:%d: %s: ", file, line, current_test);
    vprintf(fmt, args);
    putchar('\n');
}

static void report(const char *file, int line, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vreport(file, line, fmt, args);
    va_end(args);
}

void run_test(const char *name, void (*fn)(void))
{
    int failed_before = checks_failed;

    current_test = name;
    tests_run++;
    fn();

    if (checks_failed == failed_before) {
        printf("--- PASS %s\n", name);
    } else {
        tests_failed++;
        printf("--- FAIL %s\n", name);
    }
}

void check_str(const char *got, const char *want, const char *file, int line)
{
    checks_run++;

    if (got == want) {
        return;
    }
    if (got != NULL && want != NULL && strcmp(got, want) == 0) {
        return;
    }

    report(file, line, "got \"%s\" want \"%s\"", got ? got : "(null)",
           want ? want : "(null)");
}

void check_mem(const void *got, const void *want, size_t n, const char *file,
               int line)
{
    checks_run++;

    if (got == want) {
        return;
    }
    if (got != NULL && want != NULL && memcmp(got, want, n) == 0) {
        return;
    }

    report(file, line, "memory differs over %zu byte(s)", n);
}

void check_int(long long got, long long want, const char *file, int line)
{
    checks_run++;

    if (got == want) {
        return;
    }

    report(file, line, "got %lld want %lld", got, want);
}

void check_double(double got, double want, double tolerance, const char *file,
                  int line)
{
    checks_run++;

    if (fabs(got - want) <= tolerance) {
        return;
    }

    report(file, line, "got %.9g want %.9g (tolerance %g)", got, want,
           tolerance);
}

void check_true(int condition, const char *expr, const char *file, int line)
{
    checks_run++;

    if (condition) {
        return;
    }

    report(file, line, "expected true: %s", expr);
}

void check_fail(const char *file, int line, const char *fmt, ...)
{
    va_list args;

    checks_run++;
    va_start(args, fmt);
    vreport(file, line, fmt, args);
    va_end(args);
}

int test_summary(void)
{
    printf("\n%d test(s), %d check(s), %d failed\n", tests_run, checks_run,
           checks_failed);

    if (checks_failed == 0) {
        printf("PASS\n");
        return 0;
    }

    printf("FAIL\n");
    return 1;
}
