# IO and sorting

**[You can find all the code for this chapter here](io/)**

Two requirements landed at once: the league must survive a server restart, and
the league table must be sorted with the leader at the top. The first is about
file IO; the second is about `qsort`. Both are small, and both are the kind of
thing Go's standard library quietly does for you.

## Persisting: choose a format that fits your tools

We already have a JSON encoder and a flat-object parser. Rather than write an
array parser, store one JSON object per line — **JSON Lines**:

```text
{"name":"Pepper","wins":10}
{"name":"Floyd","wins":7}
```

Now loading is a loop over lines, and each line goes straight through
`json_get_string` and `json_get_int`, which we already tested. The format
choice made the parser trivial.

### Write the test first

```c
static void test_save_and_load_round_trip(void)
{
    char path[] = "/tmp/mctest-ioXXXXXX";
    int fd = mkstemp(path);
    struct player_store saved;
    struct player_store loaded;
    int wins = 0;

    CHECK_TRUE(fd >= 0);
    if (fd >= 0) {
        close(fd);
    }

    player_store_init(&saved);
    player_store_record_win(&saved, "Pepper");
    player_store_record_win(&saved, "Pepper");
    player_store_record_win(&saved, "Floyd");

    CHECK_TRUE(player_store_save(&saved, path));

    player_store_init(&loaded);
    CHECK_TRUE(player_store_load(&loaded, path));

    CHECK_TRUE(player_store_get_wins(&loaded, "Pepper", &wins));
    CHECK_INT(wins, 2);

    player_store_free(&saved);
    player_store_free(&loaded);
    unlink(path);
}
```

### Write enough code to make it pass

```c
bool player_store_save(const struct player_store *store, const char *path)
{
    FILE *f = fopen(path, "w");
    bool ok = true;

    if (f == NULL) {
        return false;
    }

    for (size_t i = 0; i < store->len; i++) {
        char line[256];

        json_encode_player(&store->players[i], line, sizeof line);
        if (fprintf(f, "%s\n", line) < 0) {
            ok = false;
            break;
        }
    }

    if (fclose(f) != 0) {
        ok = false;
    }
    return ok;
}
```

Two habits show up here that we have built over the whole book:

- **Check `fclose`.** Closing can fail — a full disk is the classic case — and
  a write that is never flushed is data you think you saved but did not. This
  is exactly the kind of thing C will not do for you.
- **Test the missing-file case.** A fresh install has no data file. Loading it
  should give an empty store, not an error:

```c
CHECK_TRUE(player_store_load(&store, "/nonexistent/mctest-io"));
CHECK_INT(store.len, 0);
```

## Sorting with `qsort`

C's sort is `qsort`, from `<stdlib.h>`. It is generic over element size and
takes a comparison function:

```c
static int compare_by_wins_desc(const void *a, const void *b)
{
    const struct player *left = a;
    const struct player *right = b;

    return right->wins - left->wins;
}

void player_store_sort_by_wins(struct player_store *store)
{
    qsort(store->players, store->len, sizeof store->players[0],
          compare_by_wins_desc);
}
```

The comparator receives `const void *`, so the first job is always to cast to
the real type. Returning `right - left` puts the higher value first — descending
order. (One caveat: subtracting two `int`s can overflow for extreme values. For
a win count it is fine; for untrusted data, compare explicitly and return
`-1`, `0`, or `1`.)

Test it directly:

```c
player_store_sort_by_wins(&store);
CHECK_STR(store.players[0].name, "Pepper");
CHECK_INT(store.players[0].wins, 3);
CHECK_STR(store.players[1].name, "Floyd");
```

## Wiring persistence into the server

The handlers need to know where the file is and whether to persist at all. We
carry that in an application context, which is what `void *ctx` was always
going to become:

```c
struct app_context {
    struct player_store store;
    bool persist;
    char store_path[256];
};
```

After a `POST`, the handler saves:

```c
if (app->persist) {
    player_store_save(&app->store, app->store_path);
}
```

And `GET /league` sorts before encoding:

```c
player_store_sort_by_wins(&app->store);
json_encode_league(&app->store, body, sizeof body);
```

This mutates the store's order as a side effect of a GET, which is a little
impure. It is harmless here because nothing depends on insertion order, but a
tidier version would sort a copy. Worth noticing rather than copying blindly.

## The acceptance test: survive a restart

This is the test that proves the requirement. Start the server with a store
file, record a win, kill it, start it again, and read the win back:

```c
port = start_and_read_port(server_path, store, &pid);
/* POST {"wins":3} ... */
CHECK_TRUE(accept_terminate(pid, 2000, &status));

/* Restart with the same store. */
port = start_and_read_port(server_path, store, &pid);
/* GET /players/Pepper should show "wins":3 */
```

Nothing else in the suite can prove persistence. A unit test of `save` and
`load` proves the functions are correct; only this proves the *program*
actually calls them, in the right order, across a real process boundary.

## Refactor

The save format is line-oriented, so two things to keep in mind if it grows:

- A name containing a newline would break the format. Our encoder escapes
  quotes but not newlines. For real data you would escape control characters
  too, or switch to a length-prefixed format.
- Saving the whole file on every `POST` is O(n) per write. Fine for a league;
  wrong for a high-traffic service, which would append instead.

We are choosing the simple thing, and — crucially — *writing down* why. That is
the difference between a shortcut and a bug.

## Wrapping up

What we have covered:

- JSON Lines as a format that makes loading easy with the tools we already have
- `fopen`/`fprintf`/`fclose`, and why checking `fclose` matters
- Treating a missing file as an empty store
- `qsort` and comparison functions over `const void *`
- Carrying configuration in an `app_context`
- An acceptance test for persistence across a process restart

### Additional material

- [`qsort` man page](https://man7.org/linux/man-pages/man3/qsort.3.html)
- [JSON Lines](https://jsonlines.org/)
