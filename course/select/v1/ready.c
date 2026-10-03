#include "ready.h"

#include <poll.h>

enum ready wait_ready(int fd_a, int fd_b, int timeout_ms)
{
    struct pollfd fds[2] = {
        {.fd = fd_a, .events = POLLIN, .revents = 0},
        {.fd = fd_b, .events = POLLIN, .revents = 0},
    };

    int ready = poll(fds, 2, timeout_ms);
    if (ready < 0) {
        return READY_ERROR;
    }
    if (ready == 0) {
        return READY_NONE;
    }

    if (fds[0].revents & POLLIN) {
        return READY_FIRST;
    }
    if (fds[1].revents & POLLIN) {
        return READY_SECOND;
    }

    return READY_ERROR;
}
