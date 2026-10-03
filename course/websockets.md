# WebSockets

**[You can find all the code for this chapter here](websockets/)**

The last requirement is real-time: a table wants to watch the league update
without refreshing. HTTP does not do that — the client asks and the server
answers, once. WebSockets start life as an HTTP request and then stop being
HTTP, turning the same socket into a two-way stream of frames. It is the
perfect capstone, because implementing it requires almost everything we have
built: sockets, HTTP, hashing, Base64, and careful binary parsing.

## How the upgrade works

A WebSocket connection begins with an ordinary HTTP request carrying a magic
header:

```text
GET /ws HTTP/1.1
Host: localhost
Upgrade: websocket
Connection: Upgrade
Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==
Sec-WebSocket-Version: 13
```

The server takes that key, appends a fixed **GUID**:

```text
258EAFA5-E914-47DA-95CA-C5AB0DC85B11
```

hashes the result with **SHA-1**, and Base64-encodes the 20-byte digest. That
value is the `Sec-WebSocket-Accept` header of the `101 Switching Protocols`
response. It exists so that a client knows it is talking to something that
really understands WebSockets rather than a proxy that blindly echoed the
headers.

So we need two primitives C does not give us: SHA-1 and Base64.

## Test the primitives against known vectors

Never write a hash function and trust it. Test it against published vectors.

```c
sha1((const unsigned char *)"abc", 3, digest);
to_hex(digest, 20, hex);
CHECK_STR(hex, "a9993e364706816aba3e25717850c26c9cd0d89d");

sha1((const unsigned char *)"", 0, digest);
to_hex(digest, 20, hex);
CHECK_STR(hex, "da39a3ee5e6b4b0d3255bfef95601890afd80709");
```

And the handshake itself has a vector in the RFC:

```c
websocket_accept_key("dGhlIHNhbXBsZSBub25jZQ==", accept, sizeof accept);
CHECK_STR(accept, "s3pPLMBiTxaQ9kYGzzhZRbK+xOo=");
```

That single assertion proves the whole chain — GUID concatenation, SHA-1, and
Base64 — is exactly right. If any of the three is wrong, it fails.

> This is why test vectors matter. You cannot look at a SHA-1 implementation
> and see whether it is correct; the algorithm is a wall of bit-twiddling. You
> compare against a value that is known to be right.

## Frames

After the upgrade, everything is **frames**. A frame is a small binary header
followed by a payload:

- byte 0: `FIN` bit and a 4-bit **opcode** (`0x1` text, `0x8` close)
- byte 1: a **mask** bit and a 7-bit length
- if the length is 126 or 127, more length bytes
- if masked, a 4-byte masking key
- the payload, XOR-masked if the mask bit is set

**Clients must mask; servers must not.** Masking is a network-security measure:
it stops poorly-written intermediaries from being fooled by frames that look
like HTTP. It is not encryption — the key travels in the clear — and the XOR is
trivial:

```c
if (masked) {
    for (uint64_t i = 0; i < len; i++) {
        payload[i] ^= (char)mask[i % 4];
    }
}
```

Writing an encoder and decoder for this is a good exercise in binary protocols:
lengths that grow, a bit that changes the meaning of the rest of the header,
and a byte-order concern in the 64-bit length.

Test the round trip, building a masked frame by hand the way a browser would:

```c
frame[n++] = 0x81;                      /* FIN + text */
frame[n++] = 0x80 | strlen(message);    /* masked, length */
memcpy(frame + n, mask, 4);
n += 4;
for (i ...) frame[n++] = message[i] ^ mask[i % 4];

write(pipefd[1], frame, n);
CHECK_INT(websocket_read_frame(pipefd[0], payload, sizeof payload), 0x1);
CHECK_STR(payload, "Hello");
```

A pipe is a perfect stand-in for a socket here: the decoder does not care what
kind of file descriptor it reads from, so we can test it without a network.

## The server

The server reads the upgrade request, extracts the key, and answers `101`:

```c
websocket_accept_key(keybuf, accept, sizeof accept);
snprintf(response, sizeof response,
         "HTTP/1.1 101 Switching Protocols\r\n"
         "Upgrade: websocket\r\n"
         "Connection: Upgrade\r\n"
         "Sec-WebSocket-Accept: %s\r\n\r\n",
         accept);
write(conn, response, strlen(response));
```

Then it loops, reading frames and echoing text ones back — a chat server in
miniature:

```c
for (;;) {
    char payload[1024];
    int opcode = websocket_read_frame(conn, payload, sizeof payload);

    if (opcode < 0) {
        break;
    }
    if (opcode == 0x8) {           /* close */
        websocket_write_close(conn);
        break;
    }
    if (opcode == 0x1) {           /* text */
        websocket_write_text(conn, payload, strlen(payload));
    }
}
```

Notice the connection stays open across many frames. This is the one place in
the whole book where a connection outlives a single request, and it is why
WebSockets needed their own loop rather than reusing `http_serve_connection`.

## The acceptance test

The test performs a real handshake and a real echo, byte for byte:

```c
const char *handshake =
    "GET /ws HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Upgrade: websocket\r\n"
    "Connection: Upgrade\r\n"
    "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
    "Sec-WebSocket-Version: 13\r\n\r\n";
write(fd, handshake, strlen(handshake));

read_until(fd, response, sizeof response, "\r\n\r\n");
CHECK_TRUE(strstr(response, "101 Switching Protocols") != NULL);
CHECK_TRUE(strstr(response, "s3pPLMBiTxaQ9kYGzzhZRbK+xOo=") != NULL);
```

Then it builds a masked frame by hand, sends it, and reads the unmasked echo
back — `0x81`, `0x02`, `h`, `i`. If you have ever wondered what "real-time web"
actually is on the wire, this is it: two extra header bytes and an XOR.

## Refactor

The echo server is single-threaded: it handles one connection at a time. A real
server wants many tables connected at once — one thread or `poll` loop per
connection, broadcasting a frame to everyone when the league changes. We have
all the pieces from the [concurrency](concurrency.md), [select](select.md) and
[sync](sync.md) chapters; wiring them together is the exercise this capstone
leaves you.

## Wrapping up

What we have covered:

- The HTTP upgrade handshake, and why `Sec-WebSocket-Accept` exists
- Implementing SHA-1 and Base64 and testing them against known vectors
- The frame format: opcodes, masking, and variable-length headers
- Why clients mask and servers do not
- Testing a binary parser against a pipe instead of a socket
- A server loop that keeps a connection open across many frames

You have now built an HTTP server, a JSON API, persistence, a CLI, a
scheduler, and a real-time protocol, all on top of POSIX sockets and the C
standard library — and every piece is covered by tests. That is a complete,
honest picture of what "the web" is made of.

### Additional material

- [RFC 6455: The WebSocket Protocol](https://www.rfc-editor.org/rfc/rfc6455)
- [FIPS 180-1 (SHA-1) test vectors](https://csrc.nist.gov/publications/detail/fips/180/1/archive)
