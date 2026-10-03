# Command line & package structure

**[You can find all the code for this chapter here](command-line/)**

Two operational requirements: the server must be configurable from the command
line (port and store path), and there must be a small tool the product owner
can run to record a win without hand-writing `curl`. Along the way we get to
talk about how to lay out a C program that has more than one executable.

## Parsing options with `getopt`

C's standard way to parse flags is `getopt` from `<unistd.h>`. It is old and a
little quirky, but it is everywhere and it handles the fiddly parts (combined
flags, `--`, error reporting) for you.

```c
bool parse_options(int argc, char **argv, struct options *out)
{
    int opt;

    memset(out, 0, sizeof *out);
    out->port = 0;

    opterr = 0;      /* we will report errors ourselves */
    optind = 1;      /* reset, so tests can call us repeatedly */

    while ((opt = getopt(argc, argv, "p:s:")) != -1) {
        switch (opt) {
        case 'p':
            out->port = atoi(optarg);
            break;
        case 's':
            copy_str(out->store_path, sizeof out->store_path, optarg);
            out->persist = true;
            break;
        default:
            return false;
        }
    }

    return true;
}
```

The `"p:s:"` string is the options spec: `p` and `s`, and a colon after each
means "this option takes an argument". `getopt` sets the global `optarg` to
that argument. When it has consumed all options, the global `optind` points at
the first non-option argument — which is how we pick up positional
`player=wins` seeds.

The `optind = 1` reset is the kind of thing that only matters because we
*test* the parser:

```c
static void test_port_and_store(void)
{
    char *argv[] = {"server", "-p", "8080", "-s", "/tmp/store", NULL};
    struct options options;

    CHECK_TRUE(parse_options(5, argv, &options));
    CHECK_INT(options.port, 8080);
    CHECK_TRUE(options.persist);
    CHECK_STR(options.store_path, "/tmp/store");
}
```

Testing your argument parsing directly beats testing it through a spawned
process — it is faster, and a failure tells you which flag broke.

## Project structure: many programs, one library

The `command-line` directory now builds **two** programs from one codebase:

```text
command_line_server   <- v1/main.c + options.c + handler.c
command_line_cli      <- v2/cli.c
```

Both link the shared `app` library (the HTTP server, the store, JSON,
persistence, and now an HTTP client). The server and the CLI share every piece
of domain logic and protocol code; only their `main` functions differ.

This is the C equivalent of the Go book's `cmd/` layout: put your reusable code
in libraries, keep `main` thin, and let several executables link the same
libraries. CMake already does this for us — each `add_chapter_program` is a
target, and `target_link_libraries` says what it needs.

> `main` should do three things: parse arguments, wire the pieces together, and
> run. If `main` contains logic you want to test, move that logic into a
> function and test the function.

## The CLI

`getopt` again, then an HTTP POST through our client:

```c
received = http_post_wins("127.0.0.1", port, player, wins, reply,
                          sizeof reply);
if (received < 0) {
    fprintf(stderr, "could not reach a server on port %d\n", port);
    return 1;
}
fputs(reply, stdout);
```

The client (`app/http_client.c`) is the mirror image of the server: connect,
build a request with `Content-Length`, write it, read the response until the
peer closes. Writing both sides by hand is the best way to understand that
"HTTP request" and "HTTP response" are just text over a socket.

Exit codes matter for a CLI, because scripts branch on them:

- `0` success
- `1` the server could not be reached
- `2` bad usage

That is a small, conventional contract, and scripts depend on it.

## The acceptance test

This test runs *two* programs: it starts the server, runs the CLI against it,
then checks the result over HTTP.

```c
pid_t pid = accept_spawn_capture(server_path, server_argv, &capture);
int port = accept_read_port(capture, 2000);
...
char port_string[16];
snprintf(port_string, sizeof port_string, "%d", port);
char *cli_argv[6] = {(char *)CLI_PATH, "-p", port_string, "Pepper", "2", NULL};

pid_t cli = accept_spawn(CLI_PATH, cli_argv);
CHECK_TRUE(accept_wait(cli, 3000, &cli_status));
CHECK_INT(WEXITSTATUS(cli_status), 0);
```

`CLI_PATH` is a compile definition supplied by CMake
(`CLI_PATH="$<TARGET_FILE:command_line_cli>"`), so the test always runs the CLI
it was built alongside. This is how you test a system of programs: start them
for real, point them at each other, and assert on what a user would observe.

`accept_wait` is the new helper here. `accept_terminate` sends `SIGTERM` and
waits; the CLI exits on its own, so the test just waits for it.

## Refactor

Now is a good time to re-read `main.c` in `command-line/v1`. It is doing four
jobs: parse options, load the store, seed players, and run the server loop.
Only the last is really `main`'s business. We could extract:

```c
static int run_server(struct app_context *app, int port);
```

and leave `main` as: parse, load, seed, `return run_server(...)`. We did not,
because the code is short and the intent is clear — but if it grew, that is the
first cut.

## Wrapping up

What we have covered:

- Parsing options with `getopt`, and testing the parser directly
- Positional arguments via `optind`, after the flags
- Project structure: multiple programs, one shared library, thin `main`s
- A CLI as an HTTP client, with meaningful exit codes
- An acceptance test that starts a server *and* a CLI and checks the result
- Passing a build target's path into a test with a CMake compile definition

### Additional material

- [`getopt` man page](https://man7.org/linux/man-pages/man3/getopt.3.html)
- [The `cmd/` directory convention in Go](https://github.com/golang-standards/project-layout)
