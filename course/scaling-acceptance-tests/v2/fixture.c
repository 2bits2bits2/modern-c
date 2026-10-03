#include "fixture.h"

#include <sys/wait.h>
#include <unistd.h>

#include "mcaccept.h"

bool fixture_start(struct server_fixture *f, const char *server_path)
{
    char *argv[4];
    char delay[] = "0";

    accept_socket_path(f->socket_path, sizeof f->socket_path, "scaling-v2");
    unlink(f->socket_path);

    argv[0] = (char *)server_path;
    argv[1] = f->socket_path;
    argv[2] = delay;
    argv[3] = NULL;

    f->pid = accept_spawn(server_path, argv);
    if (f->pid <= 0) {
        return false;
    }

    /* Wait for readiness by opening and closing one connection. */
    int fd = accept_connect_unix(f->socket_path, 2000);
    if (fd < 0) {
        accept_terminate(f->pid, 1000, NULL);
        return false;
    }

    close(fd);
    return true;
}

int fixture_connect(struct server_fixture *f)
{
    return accept_connect_unix(f->socket_path, 2000);
}

bool fixture_stop(struct server_fixture *f)
{
    int status = 0;
    bool exited = accept_terminate(f->pid, 2000, &status);

    unlink(f->socket_path);
    return exited && WIFEXITED(status) && WEXITSTATUS(status) == 0;
}
