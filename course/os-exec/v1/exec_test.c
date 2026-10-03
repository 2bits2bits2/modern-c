#include "exec.h"
#include "mctest.h"

static void test_echo(void)
{
    char out[64];
    char *argv[] = {"echo", "hello", NULL};

    int n = exec_run(argv, out, sizeof out);

    CHECK_INT(n, 6);
    CHECK_STR(out, "hello\n");
}

static void test_arguments_are_not_interpreted_by_a_shell(void)
{
    char out[64];
    char *argv[] = {"echo", "; rm -rf /", NULL};

    exec_run(argv, out, sizeof out);

    /* The argument is echoed literally; no shell ran the semicolon. */
    CHECK_STR(out, "; rm -rf /\n");
}

static void test_failed_command_produces_no_output(void)
{
    char out[64];
    char *argv[] = {"definitely-not-a-command-mctest", NULL};

    CHECK_INT(exec_run(argv, out, sizeof out), 0);
    CHECK_STR(out, "");
}

int main(void)
{
    RUN_TEST(test_echo);
    RUN_TEST(test_arguments_are_not_interpreted_by_a_shell);
    RUN_TEST(test_failed_command_produces_no_output);
    return test_summary();
}
