# Revisiting HTTP handlers

**[You can find all the code for this chapter here](http-handlers-revisited/)**

Testing HTTP handlers is the bane of many a developer's existence. Our
[HTTP server](http-server.md) chapter already solved the big problem — the
handler is a plain function over structs, so it needs no socket to test — but
the handler itself has grown into a chain of `if`s. This chapter revisits the
design: a **router table** and **middleware**.

## The problem with a growing chain

By the [JSON](json.md) chapter, `app_handler` looked like this:

```c
if (strcmp(request->method, "GET") == 0) {
    if (name != NULL) { get_player(...); }
    else if (strcmp(request->path, "/league") == 0) { get_league(...); }
    else { /* 404 */ }
    return;
}
if (strcmp(request->method, "POST") == 0) { ... }
/* 405 */
```

It works. But adding a route means editing control flow, the method check is
duplicated, and a 405 is easy to forget. Routing is data — a table of
(method, path pattern, handler) — and data belongs in a table.

## A router table

```c
struct route {
    const char *method;
    const char *pattern; /* exact, or with one {name} segment */
    http_handler handler;
};

struct router {
    const struct route *routes;
    size_t count;
    void *ctx;
};
```

The handler becomes a declaration:

```c
static const struct route routes[] = {
    {"GET", "/players/{name}", get_player},
    {"POST", "/players/{name}", post_player},
    {"GET", "/league", get_league},
};
```

And the router does the matching, including the `{name}` capture for free.

## Write the test first

Routing is tested by calling the handler directly — no socket in sight:

```c
static void test_route_with_parameter(void)
{
    struct app_context app;
    struct http_request request;
    struct http_response response;

    memset(&app, 0, sizeof app);
    player_store_init(&app.store);
    player_store_record_win(&app.store, "Pepper");

    make_request("GET", "/players/Pepper", NULL, &request);
    app_handler(&request, &response, &app);

    CHECK_INT(response.status, 200);
    CHECK_STR(response.body, "{\"name\":\"Pepper\",\"wins\":1}");

    player_store_free(&app.store);
}
```

And the two cases that the old chain got wrong are now guaranteed by the
router itself:

```c
make_request("GET", "/nope", NULL, &request);
app_handler(&request, &response, &app);
CHECK_INT(response.status, 404);

make_request("DELETE", "/players/Pepper", NULL, &request);
app_handler(&request, &response, &app);
CHECK_INT(response.status, 405); /* path exists, method does not */
```

## Write enough code to make it pass

The router's matching is the interesting part. It supports an exact pattern
(strcmp) or a single `{name}` segment (prefix, suffix, and the value in
between):

```c
if (brace != NULL) {
    close = strchr(brace, '}');
    prefix_len = (size_t)(brace - pattern);
    if (strncmp(pattern, path, prefix_len) != 0) return false;

    value_start = path + prefix_len;
    suffix = close + 1;
    suffix_len = strlen(suffix);
    path_len = strlen(value_start);
    if (path_len < suffix_len) return false;
    if (strcmp(value_start + (path_len - suffix_len), suffix) != 0) return false;
    /* value is value_start, value_len bytes long */
}
```

The captured value is written onto `request->path_param` before the handler is
called, so `get_player` reads `request->path_param` instead of parsing the path
itself. That is a small, honest bit of shared state, and it keeps every handler
simple.

If a path matches but no method does, the router answers `405`; if nothing
matches, `404`. The status codes stop being something each handler must
remember.

> **A note on modifying earlier chapters.** Adding `path_param` to
> `struct http_request` is a *compatible* change: existing code keeps compiling.
> Extending a struct is one of the few kinds of change you can make to a shared
> library in C without breaking its users — which is exactly why we put the
> application in `app/` and add to it.

## Middleware

Once every request goes through one function, you have a single place to add
cross-cutting behaviour: logging, timing, authentication, turning errors into
responses. That is middleware.

A middleware wraps a handler. The C version is concrete rather than clever —
it holds the next handler and a log function:

```c
struct logged_handler {
    http_handler next;
    void *next_ctx;
    void (*log)(void *log_ctx, const char *method, const char *path,
                int status);
    void *log_ctx;
};

void logged_handle(const struct http_request *request,
                   struct http_response *response, void *ctx)
{
    struct logged_handler *logged = ctx;

    logged->next(request, response, logged->next_ctx);
    logged->log(logged->log_ctx, request->method, request->path,
                response->status);
}
```

Because both the next handler and the log function are injected, the middleware
is a normal, testable unit:

```c
static void test_logs_after_the_handler_runs(void)
{
    struct spy_log spy = {0};
    struct logged_handler logged = {
        .next = ok_handler, .next_ctx = NULL,
        .log = spy_log_fn, .log_ctx = &spy,
    };
    /* ... call logged_handle ... */

    CHECK_INT(spy.calls, 1);
    CHECK_STR(spy.method, "GET");
    CHECK_STR(spy.path, "/players/Pepper");
    CHECK_INT(spy.status, 200);
}
```

No socket, no server, no log file. The spy proves the middleware logs after the
inner handler runs and reports the *final* status — which is what you want,
because a middleware that logs before the handler cannot know the status.

In the server, the middleware wraps the router:

```c
struct logged_handler logged = {
    .next = app_handler, .next_ctx = &app,
    .log = log_request, .log_ctx = NULL,
};
/* ... */
http_serve_connection(conn, logged_handle, &logged);
```

Chaining more middleware is the same idea applied again. In Go this is
`func(http.Handler) http.Handler`; in C it is a struct holding the next handler
and whatever the middleware needs. Neither is magic.

## Wrapping up

What we have covered:

- Routing as a data table instead of a chain of `if`s
- Matching exact paths and one `{name}` segment
- Centralising `404` and `405` in the router
- Extending a struct compatibly, and why that matters for a shared library
- Middleware: a handler that wraps another handler
- Testing middleware with a spy — no server, no socket

Handlers are testable in C for the same reason they are testable in Go: if a
handler is a function of a request and a response, you can test it directly.
Keep the network at the edges, keep the logic in the middle, and the "bane of
many a developer's existence" turns out to be ordinary code.

### Additional material

- [Go `http.Handler` and middleware](https://pkg.go.dev/net/http#Handler)
- [REST API status codes](https://developer.mozilla.org/en-US/docs/Web/HTTP/Status)
