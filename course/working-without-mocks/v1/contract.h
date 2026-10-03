#ifndef CONTRACT_H
#define CONTRACT_H

#include "user_store.h"

/*
 * Every implementation of user_store must satisfy this contract. Run it from
 * each implementation's test suite.
 */
void test_store_contract(struct user_store store);

#endif /* CONTRACT_H */
