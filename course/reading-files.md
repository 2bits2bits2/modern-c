# Reading files

**[You can find all the code for this chapter here](reading-files/)**

We need to read a file into memory. It is a small function with a surprising
number of ways to get it wrong: forgetting to close the file, not allocating
room for the terminator, assuming the whole file fits in an `int`, and leaks on
the error paths. Let's TDD it carefully.

## Write the test first

The test has to create a file to read. `mkstemp` gives us a unique temporary
file safely:

```c
static void test_read_file(void)
{
    char path[] = "/tmp/mctest-readXXXXXX";
    int fd = mkstemp(path);
    CHECK_TRUE(fd >= 0);

    const char *content = "hello\nworld\n";
    CHECK_INT(write(fd, content, 12), 12);
    close(fd);

    size_t len = 0;
    char *data = read_file(path, &len);

    CHECK_TRUE(data != NULL);
    CHECK_STR(data, content);
    CHECK_INT(len, 12);

    free(data);
    unlink(path);
}
```

> Note `mkstemp` needs a writable array of characters (it modifies the
> `XXXXXX`), which is why `path` is `char path[]`, not `const char *path`.

And the failure case, because a function that returns a pointer must have a way
to say "no":

```c
static void test_missing_file(void)
{
    CHECK_TRUE(read_file("/nonexistent/mctest-nope", NULL) == NULL);
}
```

Notice we pass `NULL` for the length. The function must tolerate that.

## Write enough code to make it pass

```c
char *read_file(const char *path, size_t *out_len)
{
    FILE *f = fopen(path, "rb");
    if (f == NULL) {
        return NULL;
    }

    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return NULL;
    }

    long size = ftell(f);
    if (size < 0) {
        fclose(f);
        return NULL;
    }
    rewind(f);

    char *buf = malloc((size_t)size + 1);
    if (buf == NULL) {
        fclose(f);
        return NULL;
    }

    size_t got = fread(buf, 1, (size_t)size, f);
    fclose(f);

    if (got != (size_t)size) {
        free(buf);
        return NULL;
    }

    buf[got] = '\0';
    if (out_len != NULL) {
        *out_len = got;
    }
    return buf;
}
```

Why `"rb"` and not `"r"`? The `b` requests binary mode. On Linux it makes no
difference, but on Windows it stops the C library translating `\r\n` into
`\n`, which would silently change your data and your byte count. Writing `"rb"`
makes the code mean the same thing everywhere.

Every error path frees what it has allocated and closes the file. There are
four `return NULL`s, and each one cleans up. This is the kind of bookkeeping
that AddressSanitizer is very good at checking for you — write a test that
triggers the `fread`-short branch (hard to do from a normal file) and the leak
would be caught.

`size + 1` reserves space for the NUL terminator. Off-by-one here is one of the
most common C bugs, and `fread` will happily write a full `size` bytes, so the
`+ 1` is not optional.

## Refactor: not every stream can seek

The `fseek`-to-the-end approach is elegant but it only works on **seekable**
streams. A pipe, a socket, or standard input cannot seek, and `ftell` will
fail. If `read_file` might ever be pointed at `stdin`, it needs a different
strategy: read in fixed-size chunks into a growable buffer until `fread`
returns short or zero.

The chunked approach is also more robust for files that change size between the
`ftell` and the `fread`. For a local file it is overkill; for general input it
is the correct answer.

For reading a file **line by line**, the standard tool is `getline`:

```c
char *line = NULL;
size_t cap = 0;

while (getline(&line, &cap, f) != -1) {
    /* line includes the trailing newline */
}

free(line);
```

`getline` allocates and grows the buffer for you, which makes it a joy compared
to `fgets` with its fixed-size manual buffer. POSIX again, not ISO C, but
available everywhere you are likely to run this.

## Wrapping up

What we have covered:

- Reading a whole file with `fopen`, `fseek`/`ftell`, `fread`
- `"rb"` so behaviour does not change across platforms
- Reserving `size + 1` for the terminator
- Cleaning up on *every* error path, not just the happy one
- Why seekable-stream code breaks on pipes, and the chunked alternative
- `getline` for line-oriented input

I/O is where C's manual memory management meets the real world, and where
habits matter. If you write every function with "who frees this, and on which
path?" in your head, you will write C that does not leak.

### Additional material

- [`getline` man page](https://man7.org/linux/man-pages/man3/getline.3.html)
- [`fopen` and friends](https://en.cppreference.com/w/c/io)
