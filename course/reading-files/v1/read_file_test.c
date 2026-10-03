#include <stdlib.h>
#include <unistd.h>

#include "mctest.h"
#include "read_file.h"

static void test_read_file(void)
{
    char path[] = "/tmp/mctest-readXXXXXX";
    int fd = mkstemp(path);
    CHECK_TRUE(fd >= 0);

    const char *content = "hello\nworld\n";
    CHECK_INT(write(fd, content, 12), 12);
    close(fd);

    size_t len = 0;
    char *data = read_file(path, &len);

    CHECK_TRUE(data != NULL);
    CHECK_STR(data, content);
    CHECK_INT(len, 12);

    free(data);
    unlink(path);
}

static void test_missing_file(void)
{
    CHECK_TRUE(read_file("/nonexistent/mctest-nope", NULL) == NULL);
}

int main(void)
{
    RUN_TEST(test_read_file);
    RUN_TEST(test_missing_file);
    return test_summary();
}
