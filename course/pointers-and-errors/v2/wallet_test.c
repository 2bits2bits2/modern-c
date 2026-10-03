#include "mctest.h"
#include "wallet.h"

static void test_withdraw(void)
{
    struct wallet w = {.balance = 20};

    CHECK_INT(wallet_withdraw(&w, 5), WALLET_OK);
    CHECK_INT(wallet_balance(&w), 15);
}

static void test_withdraw_insufficient_funds_leaves_balance_alone(void)
{
    struct wallet w = {.balance = 20};

    CHECK_INT(wallet_withdraw(&w, 100), WALLET_ERR_INSUFFICIENT_FUNDS);
    CHECK_INT(wallet_balance(&w), 20);
    CHECK_STR(wallet_err_string(WALLET_ERR_INSUFFICIENT_FUNDS),
              "insufficient funds");
}

static void test_withdraw_negative(void)
{
    struct wallet w = {.balance = 20};

    CHECK_INT(wallet_withdraw(&w, -5), WALLET_ERR_NEGATIVE_AMOUNT);
    CHECK_INT(wallet_balance(&w), 20);
}

int main(void)
{
    RUN_TEST(test_withdraw);
    RUN_TEST(test_withdraw_insufficient_funds_leaves_balance_alone);
    RUN_TEST(test_withdraw_negative);
    return test_summary();
}
