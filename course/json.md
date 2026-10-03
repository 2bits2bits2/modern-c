# JSON, routing and embedding

**[You can find all the code for this chapter here](json/)**

The product owner wants other teams to consume the league data, so the API
needs to speak JSON. In Go you would reach for `encoding/json` and, for
serving static files, `//go:embed`. C has neither, so we write a small JSON
encoder, a minimal parser for request bodies, and a router. This is a
surprisingly good use of an afternoon.

## Write the test first: the encoder

```c
static void test_encode_league(void)
{
    struct player_store store;
    char buf[256];

    player_store_init(&store);
    player_store_record_win(&store, "Pepper");
    player_store_record_win(&store, "Pepper");
    player_store_record_win(&store, "Floyd");

    json_encode_league(&store, buf, sizeof buf);

    CHECK_STR(buf, "[{\"name\":\"Pepper\",\"wins\":2},"
                    "{\"name\":\"Floyd\",\"wins\":1}]");

    player_store_free(&store);
}
```

## Write enough code to make it pass

Writing JSON is mostly string building, with one thing you must not forget:
**escaping**. A name containing a quote would produce invalid JSON if we wrote
it raw.

```c
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
```

There is a test for exactly this, because it is exactly the kind of thing that
"works" until a customer has a quote in their name:

```c
player_store_record_win(&store, "a\"b");
json_encode_league(&store, buf, sizeof buf);
CHECK_STR(buf, "[{\"name\":\"a\\\"b\",\"wins\":1}]");
```

The encoder builds into a caller-supplied buffer, so there is no allocation to
free and no way to overflow it — every append is bounds-checked. That is the
trade: a maximum response size, in exchange for simple, leak-free code.

## The minimal parser

The service also needs to read JSON bodies. A full JSON parser is a chapter of
its own; a flat-object parser is not. We need "find `"key"`, skip to its value,
read a string or an integer":

```c
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
```

`find_value` scans for a quoted key, and if the next non-space character is a
colon, returns the start of the value. It handles strings and numbers, which is
all our endpoints accept.

> **Know the edges of your parser.** This one does not handle nested objects,
> arrays, or escapes inside keys. Rather than let it silently mis-read a
> complex body, it returns `false` and the handler responds `400`. A small
> parser that fails loudly is better than a large one that fails subtly.

## Routing with path parameters

We already had `/players/{name}`; now we add `/league`, and a `POST` on the
player route. Routing is a chain of conditions, but we keep each route in its
own function so the top-level handler reads like a table:

```c
void app_handler(const struct http_request *request,
                 struct http_response *response, void *ctx)
{
    struct player_store *store = ctx;
    const char *name = player_name(request->path);

    if (strcmp(request->method, "GET") == 0) {
        if (name != NULL) {
            get_player(store, name, response);
        } else if (strcmp(request->path, "/league") == 0) {
            get_league(store, response);
        } else {
            http_response_json(response, 404, "{\"error\":\"not found\"}");
        }
        return;
    }

    if (strcmp(request->method, "POST") == 0) {
        if (name == NULL) {
            http_response_json(response, 404, "{\"error\":\"not found\"}");
            return;
        }
        post_player(store, request, name, response);
        return;
    }

    http_response_json(response, 405, "{\"error\":\"method not allowed\"}");
}
```

`GET /players/Pepper` now returns `{"name":"Pepper","wins":3}`, not `3`. The
acceptance test for the old text response would fail, which is correct — the
contract changed, and the test should be updated deliberately.

### The POST handler

```c
static void post_player(struct player_store *store,
                        const struct http_request *request, const char *name,
                        struct http_response *response)
{
    long wins = 1;

    if (request->body_len > 0) {
        if (!json_get_int(request->body, "wins", &wins) || wins < 0) {
            http_response_json(response, 400, "{\"error\":\"bad request body\"}");
            return;
        }
    }

    for (long i = 0; i < wins; i++) {
        player_store_record_win(store, name);
    }

    http_response_json(response, 201, "{\"status\":\"recorded\"}");
}
```

A body is optional (`wins` defaults to 1), and a body we cannot parse is a
`400` before any state changes. Test both, plus the `405` for an unsupported
method:

```c
static void test_post_rejects_bad_body(void)
{
    /* ... */
    make_request("POST", "/players/Pepper", "{\"wins\":\"lots\"}", &request);
    app_handler(&request, &response, &store);
    CHECK_INT(response.status, 400);
}
```

That test is a nice example of testing the failure mode: `"lots"` is not a
number, so `json_get_int` fails, and we never touch the store.

## Acceptance: record then read

The acceptance test sends a real `POST` with a body and a `Content-Length`,
then reads the player back:

```c
accept_roundtrip(fd,
                 "POST /players/Pepper HTTP/1.1\r\n"
                 "Host: localhost\r\n"
                 "Content-Length: 10\r\n"
                 "Connection: close\r\n\r\n"
                 "{\"wins\":3}",
                 reply, sizeof reply);
CHECK_TRUE(strstr(reply, "201 Created") != NULL);
```

Look at the blank line between the headers and `{"wins":3}`. That blank line is
the whole of the HTTP "here comes the body" convention. Once you have typed it
yourself, `curl -d '{"wins":3}'` stops being magic.

## On "embedding"

The Go version of this chapter also covers `//go:embed`, which bakes files into
the binary. C's equivalent is a build step that turns a file into a C array:

```sh
xxd -i index.html > index_html.c
```

which gives you `unsigned char index_html[]` and `unsigned int index_html_len`,
ready to compile in. There is no code for it here because it is a *build*
feature, not a language one — and because doing it by hand makes clear that
"embedding" is just bytes in your `.rodata`.

## Wrapping up

What we have covered:

- Encoding JSON by hand, including string escaping
- Building into a bounded caller buffer: no allocation, no overflow
- A minimal, honest flat-object parser, and failing loudly at its edges
- Routing by method and path, with each route in its own function
- `POST` bodies with `Content-Length`, and returning `400` before mutating
- That `//go:embed` is a build trick you can reproduce with `xxd -i`

### Additional material

- [JSON specification (RFC 8259)](https://www.rfc-editor.org/rfc/rfc8259)
- [`xxd`](https://linux.die.net/man/1/xxd)
