#include <stdlib.h>
#include <unistd.h>

#include "contract.h"
#include "mctest.h"
#include "user_store.h"

static void test_memory_store_contract(void)
{
    struct user_store store = memory_store_new();

    test_store_contract(store);

    memory_store_free(store);
}

static void test_file_store_contract(void)
{
    char path[] = "/tmp/mctest-storeXXXXXX";
    int fd = mkstemp(path);

    CHECK_TRUE(fd >= 0);
    if (fd >= 0) {
        close(fd);
    }
    /* Start empty so the contract sees a blank slate. */
    unlink(path);

    struct user_store store = file_store_new(path);
    test_store_contract(store);
    file_store_free(store);

    unlink(path);
}

int main(void)
{
    RUN_TEST(test_memory_store_contract);
    RUN_TEST(test_file_store_contract);
    return test_summary();
}
