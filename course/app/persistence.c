#include "persistence.h"

#include <stdio.h>
#include <string.h>

#include "json.h"

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

bool player_store_load(struct player_store *store, const char *path)
{
    FILE *f = fopen(path, "r");
    char line[256];

    if (f == NULL) {
        return true; /* no file yet: start empty */
    }

    while (fgets(line, sizeof line, f) != NULL) {
        char name[PLAYER_NAME_MAX];
        long wins = 0;

        if (!json_get_string(line, "name", name, sizeof name) ||
            !json_get_int(line, "wins", &wins)) {
            continue;
        }

        for (long i = 0; i < wins; i++) {
            player_store_record_win(store, name);
        }
    }

    fclose(f);
    return true;
}
