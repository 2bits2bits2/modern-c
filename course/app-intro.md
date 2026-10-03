# A new application: the player store

This is the start of a new section. Up to now every chapter has been
self-contained: you could read the code for one chapter without reading any
other. That worked because the fundamentals are small.

An application is not small. It is an HTTP server, a JSON API, a data store, a
command line, and eventually a real-time channel — and if we copied all of that
into every chapter, the duplication would bury the thing each chapter is
actually trying to teach. So this section is organised differently.

## One growing application

The application lives in `course/app/`:

```text
app/
  http.c        HTTP/1.1 request parsing, responses, a TCP listener
  player_store.c an in-memory league of players and their wins
  json.c        a JSON encoder and a minimal parser
  persistence.c saving and loading JSON Lines files
  http_client.c the client side, used by the CLI
  clock.c       an injectable clock
  scheduler.c   scheduling callbacks against that clock
  base64.c      Base64
  sha1.c        SHA-1
  websocket.c   the RFC 6455 handshake and frames
```

Each chapter **adds** to this library rather than rewriting it, so the code for
earlier chapters keeps compiling exactly as it did. The chapters themselves
(`http-server/`, `json/`, `io/`, ...) contain the parts that genuinely differ
from one chapter to the next: the request handlers, the programs, and the
tests.

That is a deliberate break from the "copy everything per version" style of the
fundamentals chapters, and it is the honest choice for a project of this size.
You will still see small, focused duplication where it helps the lesson.

## The product

Our product owner runs a card league. The requirements arrive over the next
chapters, roughly in this order:

- An HTTP service that records wins and reports a player's score
- A JSON API, so other systems can consume the league
- Persistence, so a restart does not lose the data — and a sorted league table
- A command line, so an operator can record a win without `curl`
- Scheduled blind levels, without slow, flaky time-based tests
- A WebSocket channel, so a table can watch the league update live

Each chapter starts the same way every other chapter in this book does: with a
requirement and a test.

## What you need to know already

- The [HTTP server](http-server.md) chapter builds the first endpoint.
- If you want to understand the foundation before using it, read
  `app/http.c`. It is a few hundred lines, and none of it is magic.

There is no new tooling. The same `cmake` and `ctest` commands build and run
everything:

```sh
cmake --build build
ctest --test-dir build --output-on-failure
```

On to the [HTTP server](http-server.md).
