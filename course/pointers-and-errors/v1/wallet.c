#include "wallet.h"

void wallet_deposit(struct wallet *w, int amount)
{
    w->balance += amount;
}

int wallet_balance(const struct wallet *w)
{
    return w->balance;
}
