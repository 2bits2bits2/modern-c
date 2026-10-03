#include "contract.h"
#include "mctest.h"
#include "user_store.h"

static void test_memory_store_contract(void)
{
    struct user_store store = memory_store_new();

    test_store_contract(store);

    memory_store_free(store);
}

int main(void)
{
    RUN_TEST(test_memory_store_contract);
    return test_summary();
}
