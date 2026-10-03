#ifndef WALLET_H
#define WALLET_H

struct wallet {
    int balance;
};

void wallet_deposit(struct wallet *w, int amount);
int wallet_balance(const struct wallet *w);

#endif /* WALLET_H */
