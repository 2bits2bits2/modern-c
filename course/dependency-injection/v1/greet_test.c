#include <stdlib.h>

#include "greet.h"
#include "mctest.h"

static void test_greet(void)
{
    char *buf = NULL;
    size_t size = 0;
    FILE *out = open_memstream(&buf, &size);

    greet(out, "Chris");
    fclose(out);

    CHECK_STR(buf, "Hello, Chris!");

    free(buf);
}

int main(void)
{
    RUN_TEST(test_greet);
    return test_summary();
}
