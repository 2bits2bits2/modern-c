#include "json.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void append_raw(char *buf, size_t cap, size_t *pos, const char *text)
{
    for (size_t i = 0; text[i] != '\0' && *pos + 1 < cap; i++) {
        buf[*pos] = text[i];
        (*pos)++;
    }
    buf[*pos] = '\0';
}

static void append_json_string(char *buf, size_t cap, size_t *pos,
                               const char *text)
{
    append_raw(buf, cap, pos, "\"");
    for (size_t i = 0; text[i] != '\0' && *pos + 1 < cap; i++) {
        if (text[i] == '"' || text[i] == '\\') {
            buf[(*pos)++] = '\\';
        }
        buf[(*pos)++] = text[i];
    }
    buf[*pos] = '\0';
    append_raw(buf, cap, pos, "\"");
}

static void append_int(char *buf, size_t cap, size_t *pos, int value)
{
    char number[32];

    snprintf(number, sizeof number, "%d", value);
    append_raw(buf, cap, pos, number);
}

void json_encode_player(const struct player *player, char *buf, size_t cap)
{
    size_t pos = 0;

    if (cap == 0) {
        return;
    }
    buf[0] = '\0';

    append_raw(buf, cap, &pos, "{\"name\":");
    append_json_string(buf, cap, &pos, player->name);
    append_raw(buf, cap, &pos, ",\"wins\":");
    append_int(buf, cap, &pos, player->wins);
    append_raw(buf, cap, &pos, "}");
}

void json_encode_league(const struct player_store *store, char *buf, size_t cap)
{
    size_t pos = 0;

    if (cap == 0) {
        return;
    }
    buf[0] = '\0';

    append_raw(buf, cap, &pos, "[");
    for (size_t i = 0; i < store->len; i++) {
        if (i > 0) {
            append_raw(buf, cap, &pos, ",");
        }
        append_raw(buf, cap, &pos, "{\"name\":");
        append_json_string(buf, cap, &pos, store->players[i].name);
        append_raw(buf, cap, &pos, ",\"wins\":");
        append_int(buf, cap, &pos, store->players[i].wins);
        append_raw(buf, cap, &pos, "}");
    }
    append_raw(buf, cap, &pos, "]");
}

static void skip_whitespace(const char **p)
{
    while (**p == ' ' || **p == '\t' || **p == '\n' || **p == '\r') {
        (*p)++;
    }
}

/* Find the value of a top-level "key" in a flat object. */
static const char *find_value(const char *json, const char *key)
{
    size_t key_len = strlen(key);
    const char *p = json;

    while ((p = strchr(p, '"')) != NULL) {
        const char *start = p + 1;
        const char *end = strchr(start, '"');
        size_t len;
        const char *colon;

        if (end == NULL) {
            return NULL;
        }

        len = (size_t)(end - start);
        if (len == key_len && strncmp(start, key, key_len) == 0) {
            colon = end + 1;
            skip_whitespace(&colon);
            if (*colon == ':') {
                colon++;
                skip_whitespace(&colon);
                return colon;
            }
        }

        p = end + 1;
    }

    return NULL;
}

bool json_get_string(const char *json, const char *key, char *out, size_t cap)
{
    const char *value = find_value(json, key);
    size_t pos = 0;

    if (value == NULL || *value != '"') {
        return false;
    }

    value++;
    while (*value != '\0' && *value != '"' && pos + 1 < cap) {
        if (*value == '\\' && value[1] != '\0') {
            value++;
        }
        out[pos] = *value;
        pos++;
        value++;
    }
    out[pos] = '\0';
    return true;
}

bool json_get_int(const char *json, const char *key, long *out)
{
    const char *value = find_value(json, key);
    char *end = NULL;
    long parsed;

    if (value == NULL) {
        return false;
    }

    parsed = strtol(value, &end, 10);
    if (end == value) {
        return false;
    }

    if (out != NULL) {
        *out = parsed;
    }
    return true;
}
