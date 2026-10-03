#include <stdio.h>
#include <string.h>

#include "mctest.h"
#include "runner.h"
#include "user.h"

/* A fake runner: records the command it was asked to run and returns canned
 * output. No process is spawned. */
struct fake_runner {
    char command[64];
    const char *output;
    int result;
};

static int fake_run(void *ctx, char *const argv[], char *out, size_t cap)
{
    struct fake_runner *fake = ctx;

    snprintf(fake->command, sizeof fake->command, "%s", argv[0]);
    if (fake->result < 0) {
        return fake->result;
    }

    size_t len = strlen(fake->output);
    if (len >= cap) {
        len = cap - 1;
    }
    memcpy(out, fake->output, len);
    out[len] = '\0';
    return (int)len;
}

static void test_current_user_trims_the_newline(void)
{
    struct fake_runner fake = {.output = "chris\n", .result = 6};
    struct command_runner runner = {.run = fake_run, .ctx = &fake};
    char name[64];

    CHECK_INT(current_user(runner, name, sizeof name), 0);
    CHECK_STR(name, "chris");
    CHECK_STR(fake.command, "whoami");
}

static void test_current_user_reports_failure(void)
{
    struct fake_runner fake = {.output = "", .result = -1};
    struct command_runner runner = {.run = fake_run, .ctx = &fake};
    char name[64];

    CHECK_INT(current_user(runner, name, sizeof name), -1);
}

int main(void)
{
    RUN_TEST(test_current_user_trims_the_newline);
    RUN_TEST(test_current_user_reports_failure);
    return test_summary();
}
