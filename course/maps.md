# Maps

**[You can find all the code for this chapter here](maps/)**

We need to store a score against a name. In Go you would write
`map[string]int`. C has no map in its standard library, so we are going to
build one — a hash map — and in doing so learn what a map really is.

## Write the test first

```c
#include <stdio.h>

#include "map.h"
#include "mctest.h"

static void test_put_and_get(void)
{
    struct map *m = map_new();
    int got = 0;

    CHECK_TRUE(map_put(m, "a", 1));
    CHECK_TRUE(map_get(m, "a", &got));
    CHECK_INT(got, 1);

    map_free(m);
}
```

Note the shape: `map_get` returns `bool` for "was it found", and writes the
value through an out-parameter `int *out`. That is because there is no way to
return "an `int` or nothing" in C — we already used the same pattern for
`add_checked` in the [integers](integers.md) chapter.

`map_new` returns a pointer, because the map owns heap memory, and
`map_free` gives it back. Every test that creates a map frees it.

## Try to run the test

The compiler has not seen `map_new`. The header:

```c
struct map_entry;

struct map {
    struct map_entry **buckets;
    size_t nbuckets;
    size_t len;
};

struct map *map_new(void);
void map_free(struct map *m);
bool map_put(struct map *m, const char *key, int value);
bool map_get(const struct map *m, const char *key, int *out);
```

`struct map_entry` is declared but not defined. That is fine, because we only
use a *pointer* to it in the header. The definition — what an entry actually
is — stays in the `.c` file. This is a form of encapsulation you will see
everywhere in C.

## Just enough hashing

A map needs to turn a key into an index into an array. That function is a
**hash**. We use FNV-1a, which is short, fast and good enough:

```c
static size_t hash_string(const char *s)
{
    uint64_t h = 1469598103934665603ULL;

    for (const unsigned char *p = (const unsigned char *)s; *p != '\0'; p++) {
        h ^= *p;
        h *= 1099511628211ULL;
    }

    return (size_t)h;
}
```

Two things matter here. First, we cast to `unsigned char *` so that bytes above
127 do not become negative when promoted to `int`. Second, the result is a
`size_t`, and we take it modulo the number of buckets to get an index.

Hash functions collide: two different keys can hash to the same bucket. So
each bucket holds a **linked list** of entries, and we compare keys with
`strcmp` to find the right one. That is called separate chaining.

## Write enough code to make it pass

```c
struct map_entry {
    char *key;
    int value;
    struct map_entry *next;
};

bool map_put(struct map *m, const char *key, int value)
{
    size_t idx = hash_string(key) % m->nbuckets;

    for (struct map_entry *e = m->buckets[idx]; e != NULL; e = e->next) {
        if (strcmp(e->key, key) == 0) {
            e->value = value;
            return true;
        }
    }

    struct map_entry *e = malloc(sizeof *e);
    if (e == NULL) {
        return false;
    }

    e->key = strdup(key);
    if (e->key == NULL) {
        free(e);
        return false;
    }

    e->value = value;
    e->next = m->buckets[idx];
    m->buckets[idx] = e;
    m->len++;
    return true;
}
```

This is where the ownership rules of the [arrays chapter](arrays-and-pointers.md)
pay off:

- The map **copies** the key with `strdup`. It does not assume the caller's
  string will outlive the map. If it did assume that, every caller would have
  to keep their keys alive forever — a bug factory.
- `map_free` must free both the key and the entry, walking every bucket.
- If `malloc` or `strdup` fails, we clean up whatever we already allocated and
  return `false`. No leak on the failure path.

`map_get` finds the entry and writes through `out` only if it was found, so a
failed lookup leaves the caller's variable alone.

Add a test that stores hundreds of keys and reads them all back. If your hash
or chaining is wrong, that test will find it.

## More requirements: deletion and growth

Two things are missing. We cannot delete, and the bucket array is fixed at 16:
as the map fills up, the linked lists get long and lookups get slow.

The fix is to **grow and rehash** when the map gets too crowded. A common rule
is "resize when `len` exceeds three times the number of buckets".

```c
static bool map_resize(struct map *m, size_t new_nbuckets)
{
    struct map_entry **new_buckets = calloc(new_nbuckets, sizeof *new_buckets);

    if (new_buckets == NULL) {
        return false;
    }

    for (size_t i = 0; i < m->nbuckets; i++) {
        struct map_entry *e = m->buckets[i];
        while (e != NULL) {
            struct map_entry *next = e->next;
            size_t idx = hash_string(e->key) % new_nbuckets;
            e->next = new_buckets[idx];
            new_buckets[idx] = e;
            e = next;
        }
    }

    free(m->buckets);
    m->buckets = new_buckets;
    m->nbuckets = new_nbuckets;
    return true;
}
```

Resizing is where a lot of C code gets its memory handling wrong. Notice we
save `next` *before* re-linking `e`, and we only free the old bucket array once
every entry has been moved.

Deletion is a good excuse to meet a classic C idiom, the **pointer to pointer**:

```c
bool map_delete(struct map *m, const char *key)
{
    size_t idx = hash_string(key) % m->nbuckets;
    struct map_entry **link = &m->buckets[idx];

    while (*link != NULL) {
        struct map_entry *e = *link;
        if (strcmp(e->key, key) == 0) {
            *link = e->next;
            free(e->key);
            free(e);
            m->len--;
            return true;
        }
        link = &e->next;
    }

    return false;
}
```

`link` starts as the address of the bucket pointer, then becomes the address of
an entry's `next` field. `*link = e->next` removes the entry whether it is the
first in its chain or somewhere in the middle, with no special case. Soaking up
this idiom will make a lot of linked-list code much shorter.

Add a test that puts 5,000 keys in and checks `m->nbuckets` grew past 16. Run
the whole thing under the sanitizer build.

## Wrapping up

What we have covered:

- A hash map: an array of buckets, each a linked list
- FNV-1a hashing, and why we cast to `unsigned char *`
- Taking ownership (`strdup`) versus borrowing references
- Resizing and rehashing under a load factor
- The pointer-to-pointer idiom for removing nodes from a list
- Opaque structs: declaring `struct map_entry` in the header but defining it in
  the `.c` file

Maps are a great chapter because they exercise everything: pointers, the heap,
strings, ownership and data structures. Go hands you one; now you know what is
inside the box.

### Additional material

- [FNV hash](http://www.isthe.com/chongo/tech/comp/fnv/)
- [Chaining vs open addressing](https://en.wikipedia.org/wiki/Hash_table#Collision_resolution)
