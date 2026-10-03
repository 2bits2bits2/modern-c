#ifndef READY_H
#define READY_H

enum ready {
    READY_ERROR = -1,
    READY_NONE = 0,
    READY_FIRST = 1,
    READY_SECOND = 2,
};

/*
 * Wait until one of the two file descriptors has input, or timeout_ms passes.
 * Pass -1 as the timeout to wait forever.
 */
enum ready wait_ready(int fd_a, int fd_b, int timeout_ms);

#endif /* READY_H */
