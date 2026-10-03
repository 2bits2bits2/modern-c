#include "wallet.h"

void wallet_deposit(struct wallet *w, int amount)
{
    w->balance += amount;
}

enum wallet_err wallet_withdraw(struct wallet *w, int amount)
{
    if (amount < 0) {
        return WALLET_ERR_NEGATIVE_AMOUNT;
    }
    if (amount > w->balance) {
        return WALLET_ERR_INSUFFICIENT_FUNDS;
    }

    w->balance -= amount;
    return WALLET_OK;
}

int wallet_balance(const struct wallet *w)
{
    return w->balance;
}

const char *wallet_err_string(enum wallet_err err)
{
    switch (err) {
    case WALLET_OK:
        return "ok";
    case WALLET_ERR_INSUFFICIENT_FUNDS:
        return "insufficient funds";
    case WALLET_ERR_NEGATIVE_AMOUNT:
        return "negative amount";
    }
    return "unknown error";
}
