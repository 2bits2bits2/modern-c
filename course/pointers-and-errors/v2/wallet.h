#ifndef WALLET_H
#define WALLET_H

struct wallet {
    int balance;
};

enum wallet_err {
    WALLET_OK = 0,
    WALLET_ERR_INSUFFICIENT_FUNDS,
    WALLET_ERR_NEGATIVE_AMOUNT,
};

void wallet_deposit(struct wallet *w, int amount);
enum wallet_err wallet_withdraw(struct wallet *w, int amount);
int wallet_balance(const struct wallet *w);
const char *wallet_err_string(enum wallet_err err);

#endif /* WALLET_H */
