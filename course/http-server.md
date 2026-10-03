# HTTP server

**[You can find all the code for this chapter here](http-server/)**

Time to build an application. Our product owner runs a card league and wants a
web service that records players' wins. Over the next several chapters we will
grow it from this first endpoint into a JSON API with persistence, a command
line, scheduling and WebSockets.

Every chapter in this section shares the same **application foundation** in
`course/app/`: the HTTP plumbing (`http.c`) and the in-memory player store
(`player_store.c`). We add files to it as we go; we do not rewrite it, so the
earlier chapters keep compiling exactly as they did. The chapters themselves
contain the routes, the programs and the tests.

> If you want to see what the HTTP layer does before using it, read
> `app/http.c`. It is small: parse a request line, dispatch to a handler,
> write a response. That is all an HTTP server is at this level.

## Write the test first

We start with the store, because the handler is built on it:

```c
static void test_record_and_get(void)
{
    struct player_store store;
    int wins = 0;

    player_store_init(&store);

    CHECK_TRUE(!player_store_get_wins(&store, "Pepper", &wins));

    player_store_record_win(&store, "Pepper");
    player_store_record_win(&store, "Pepper");
    player_store_record_win(&store, "Floyd");

    CHECK_TRUE(player_store_get_wins(&store, "Pepper", &wins));
    CHECK_INT(wins, 2);
    CHECK_TRUE(player_store_get_wins(&store, "Floyd", &wins));
    CHECK_INT(wins, 1);

    player_store_free(&store);
}
```

The store is a growable array of `struct player`, exactly the pattern from the
[arrays chapter](arrays-and-pointers.md). We test it directly rather than
through HTTP, because "does lookup work?" and "does routing work?" are
different questions.

## The handler

The HTTP layer hands our handler a parsed request and a response to fill in:

```c
typedef void (*http_handler)(const struct http_request *request,
                             struct http_response *response, void *ctx);
```

We route `GET /players/{name}` to the store:

```c
void app_handler(const struct http_request *request,
                 struct http_response *response, void *ctx)
{
    struct player_store *store = ctx;

    if (strcmp(request->method, "GET") == 0) {
        const char *name = player_name(request->path);

        if (name != NULL) {
            int wins = 0;

            if (player_store_get_wins(store, name, &wins)) {
                char body[32];

                snprintf(body, sizeof body, "%d", wins);
                http_response_text(response, 200, body);
                return;
            }

            http_response_text(response, 404, "player not found");
            return;
        }
    }

    http_response_text(response, 404, "not found");
}
```

`player_name` strips the `/players/` prefix. Extracting it into a function
keeps the routing readable and gives us a thing to test on its own.

### Test the handler without a network

This is the key design decision of the chapter. `app_handler` takes a
`struct http_request` and a `struct http_response` — plain structs. We do not
need a socket to test routing:

```c
static void test_get_player_score(void)
{
    struct player_store store;
    struct http_request request;
    struct http_response response;

    player_store_init(&store);
    player_store_record_win(&store, "Pepper");
    player_store_record_win(&store, "Pepper");
    player_store_record_win(&store, "Pepper");

    request_get("/players/Pepper", &request);
    app_handler(&request, &response, &store);

    CHECK_INT(response.status, 200);
    CHECK_STR(response.body, "3");
    CHECK_STR(response.content_type, "text/plain");

    player_store_free(&store);
}
```

If the handler were tangled up with `recv` and `send`, every routing test would
need a connection. Separating "what to respond" from "how to put bytes on a
socket" is what makes the bulk of an HTTP service fast and easy to test. The
network is tested once, by an acceptance test.

## The server program

The program ties the socket to the handler:

```c
int listener = http_listen_tcp(port);
...
printf("PORT=%d\n", http_local_port(listener));
fflush(stdout);

while (running) {
    int conn = accept(listener, NULL, NULL);

    if (conn < 0) {
        if (errno == EINTR) {
            continue;
        }
        break;
    }

    http_serve_connection(conn, app_handler, &store);
    close(conn);
}
```

Two things are worth calling out:

- The server prints `PORT=<n>` and flushes. When we pass `--port 0`, the OS
  picks a free port and we tell the test what it is. That is how the acceptance
  test avoids hard-coded ports and port collisions.
- `--seed Pepper=10` pre-loads a player so the acceptance test has something to
  find. It is a test affordance, and honest about it.

## The acceptance test

The test starts the real program, reads the port it chose, and speaks HTTP:

```c
static void test_get_player_score_over_http(void)
{
    char seed[] = "Pepper=10";
    char *argv[6] = {(char *)server_path, "--port", "0", "--seed", seed, NULL};

    int capture = -1;
    pid_t pid = accept_spawn_capture(server_path, argv, &capture);
    CHECK_TRUE(pid > 0);

    int port = accept_read_port(capture, 2000);
    close(capture);
    CHECK_TRUE(port > 0);

    int fd = accept_connect_tcp("127.0.0.1", port, 2000);
    CHECK_TRUE(fd >= 0);

    if (fd >= 0) {
        char reply[512];

        accept_roundtrip(fd,
                         "GET /players/Pepper HTTP/1.1\r\n"
                         "Host: localhost\r\n"
                         "Connection: close\r\n\r\n",
                         reply, sizeof reply);
        CHECK_TRUE(strstr(reply, "200 OK") != NULL);
        CHECK_TRUE(strstr(reply, "10") != NULL);
        close(fd);
    }
    /* ... and a 404 for an unknown player ... */
}
```

The request is not exotic — it is the same bytes a browser sends, just typed
out. Seeing `GET /players/Pepper HTTP/1.1` and the blank line that separates
headers from body demystifies HTTP, which is the point of building it by hand.

## Refactor

Our handler returns a bare number as `text/plain`. The next chapter's product
requirement is that the service speaks JSON, so the league table can be
consumed by other software. We will add a JSON encoder and a router, and change
`/players/{name}` to return `{"name":...,"wins":...}`.

Before that, notice how little code sits between the socket and the store
because of the split:

- `app/http.c` — protocol, no business logic
- `handler.c` — business logic, no sockets
- `main.c` — process lifecycle, no logic at all

That layering is what lets a large service stay testable as it grows. Keep it.

## Wrapping up

What we have covered:

- An HTTP/1.1 server on raw TCP sockets: request line, headers, body
- Handlers as plain functions over request/response structs, so routing is
  testable without a network
- Acceptance-testing the assembled program over a real socket
- `--port 0` plus a printed port to avoid hard-coded ports
- Layering: protocol in `app/`, logic in `handler.c`, lifecycle in `main.c`

`curl` it yourself once you have built it:

```sh
./build/http-server/http_server_v2_server --port 8080 --seed Pepper=10 &
curl -i localhost:8080/players/Pepper
```

### Additional material

- [RFC 9110: HTTP semantics](https://www.rfc-editor.org/rfc/rfc9110)
- [`accept` man page](https://man7.org/linux/man-pages/man2/accept.2.html)
