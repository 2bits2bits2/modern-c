#include "mctest.h"
#include "wallet.h"

static void test_deposit(void)
{
    struct wallet w = {.balance = 0};

    wallet_deposit(&w, 10);
    CHECK_INT(wallet_balance(&w), 10);

    wallet_deposit(&w, 5);
    CHECK_INT(wallet_balance(&w), 15);
}

static void test_a_new_wallet_is_empty(void)
{
    struct wallet w = {.balance = 0};

    CHECK_INT(wallet_balance(&w), 0);
}

int main(void)
{
    RUN_TEST(test_deposit);
    RUN_TEST(test_a_new_wallet_is_empty);
    return test_summary();
}
