#include "contract.h"

#include <string.h>

#include "mctest.h"

void test_store_contract(struct user_store store)
{
    struct user out;
    struct user alice = {.name = "alice", .age = 30};

    memset(&out, 0, sizeof out);

    CHECK_TRUE(!store.get(store.ctx, "nobody", &out));

    CHECK_TRUE(store.save(store.ctx, &alice));
    CHECK_TRUE(store.get(store.ctx, "alice", &out));
    CHECK_STR(out.name, "alice");
    CHECK_INT(out.age, 30);

    /* Saving an existing user updates them. */
    struct user older = {.name = "alice", .age = 31};
    CHECK_TRUE(store.save(store.ctx, &older));
    CHECK_TRUE(store.get(store.ctx, "alice", &out));
    CHECK_INT(out.age, 31);

    /* Other users are unaffected. */
    struct user bob = {.name = "bob", .age = 25};
    CHECK_TRUE(store.save(store.ctx, &bob));
    CHECK_TRUE(store.get(store.ctx, "bob", &out));
    CHECK_INT(out.age, 25);
    CHECK_TRUE(store.get(store.ctx, "alice", &out));
    CHECK_INT(out.age, 31);
}
