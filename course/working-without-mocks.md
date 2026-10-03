# Working without mocks, stubs and spies

**[You can find all the code for this chapter here](working-without-mocks/)**

We need to save and load users. In the real system they go into a database;
in tests we do not want a database. The Go chapter's argument, which is even
more true in C, is this: instead of building elaborate mocks that imitate the
database, define a small **interface**, write several simple implementations,
and hold them all to one **contract**.

## The interface

```c
struct user_store {
    bool (*save)(void *ctx, const struct user *u);
    bool (*get)(void *ctx, const char *name, struct user *out);
    void *ctx;
};
```

Two operations, two function pointers, a context. No dependencies. Anything
that can save and get a user is a `user_store`.

## Write the contract first

A contract is a test suite that takes *any* implementation and asserts the
rules every implementation must obey. It is not a test of one implementation;
it is a test of the interface.

```c
void test_store_contract(struct user_store store)
{
    struct user out;
    struct user alice = {.name = "alice", .age = 30};

    memset(&out, 0, sizeof out);

    /* A missing user is reported, not fabricated. */
    CHECK_TRUE(!store.get(store.ctx, "nobody", &out));

    /* What was saved can be read back. */
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
```

Every implementation calls this function from its own test. One suite,
many implementations.

To see that the contract has teeth, point it at a deliberately broken store —
one whose `save` pretends to succeed but whose `get` always says no:

```text
    contract.c:17: test_broken_store: expected true: store.get(store.ctx, "alice", &out)
    contract.c:18: test_broken_store: got "" want "alice"
    contract.c:19: test_broken_store: got 0 want 30
    ...
--- FAIL test_broken_store

1 test(s), 13 check(s), 9 failed
FAIL
```

The contract pins down the interface's meaning, and a wrong implementation
cannot sneak past it. That is a far stronger guarantee than a mock asserting
"the database was called once".

## The in-memory implementation

For tests, a growable array is plenty:

```c
static bool memory_save(void *ctx, const struct user *u)
{
    struct memory_store *m = ctx;

    for (size_t i = 0; i < m->len; i++) {
        if (strcmp(m->users[i].name, u->name) == 0) {
            m->users[i] = *u;      /* update in place */
            return true;
        }
    }

    /* ... grow, then append ... */
}
```

And the test:

```c
static void test_memory_store_contract(void)
{
    struct user_store store = memory_store_new();

    test_store_contract(store);

    memory_store_free(store);
}
```

This is a **fake**: a real, working implementation, just not the production
one. It is not a mock. It does not record calls or assert expectations. It
genuinely stores users, so the test exercises real save/get logic instead of
the shape of a mock's API.

## A second implementation: the file store

Now the payoff. Write a completely different `user_store` that keeps users in
a text file, and run the *same* contract against it:

```c
static void test_file_store_contract(void)
{
    char path[] = "/tmp/mctest-storeXXXXXX";
    int fd = mkstemp(path);

    CHECK_TRUE(fd >= 0);
    if (fd >= 0) {
        close(fd);
    }
    unlink(path);      /* start empty */

    struct user_store store = file_store_new(path);
    test_store_contract(store);
    file_store_free(store);

    unlink(path);
}
```

If the file store passes the same contract as the memory store, you have real
assurance that swapping one for the other will not change the behaviour your
code depends on. That is the whole idea, and it is worth more than any number
of mocks.

The file implementation is an append-only log: `save` writes a `name,age`
line, and `get` scans the file and keeps the last match. Append-only is simple
and, for a test fake, entirely adequate.

## Mocks, fakes, stubs, spies — when to use what

- **Fakes** (like both stores here) are best when the interface has real,
  observable behaviour. They give you confidence the logic is right.
- **Stubs** are fine when you only need an input. A function returning a fixed
  value is a stub, and writing one is trivial in C.
- **Spies** are for when the *interaction* is the contract — the countdown
  chapter's sleeper is the example. Record calls, assert on the ones that
  matter, ignore the rest.
- **Full mocks with expectation frameworks** are almost never needed in C.
  The interface is a struct of function pointers, so a test double is a struct
  of functions — a few lines, no dependency, easy to read.

The rule of thumb from the Go chapter holds: if you find yourself building an
elaborate mock, that is a signal the interface is too big. Shrink the
interface until the fake is obvious.

## Wrapping up

What we have covered:

- Defining a small interface (a struct of function pointers) so behaviour can
  be swapped
- Writing a **contract test** — one suite, many implementations
- Proving the contract has teeth by failing a broken implementation
- A **fake** (in-memory store) and a second implementation (file store) held to
  the same contract
- When to reach for a stub, a spy, or a fake, and why full mocks are rarely
  worth it in C

This is the most useful testing idea in the book. A contract turns "does this
work?" into "does this satisfy the rules?", and it lets you test expensive
dependencies — databases, networks, filesystems — without any of them being
present.

### Additional material

- [Mocks aren't stubs](https://martinfowler.com/articles/mocksArentStubs.html)
- [Contract tests](https://martinfowler.com/bliki/ContractTest.html)
