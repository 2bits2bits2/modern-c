#include "exec.h"

#include <sys/wait.h>
#include <unistd.h>

int exec_run(char *const argv[], char *out, size_t cap)
{
    int fds[2];
    size_t total = 0;
    int status = 0;
    pid_t pid;

    if (cap == 0 || pipe(fds) != 0) {
        return -1;
    }

    pid = fork();
    if (pid < 0) {
        close(fds[0]);
        close(fds[1]);
        return -1;
    }

    if (pid == 0) {
        /* Child: send stdout into the pipe, then become the program. */
        close(fds[0]);
        dup2(fds[1], STDOUT_FILENO);
        close(fds[1]);
        execvp(argv[0], argv);
        _exit(127);
    }

    /* Parent: read until the child closes the pipe. */
    close(fds[1]);
    while (total + 1 < cap) {
        ssize_t n = read(fds[0], out + total, cap - 1 - total);

        if (n < 0) {
            close(fds[0]);
            waitpid(pid, &status, 0);
            return -1;
        }
        if (n == 0) {
            break;
        }
        total += (size_t)n;
    }
    out[total] = '\0';

    close(fds[0]);
    waitpid(pid, &status, 0);
    return (int)total;
}
