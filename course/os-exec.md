# OS Exec

**[You can find all the code for this chapter here](os-exec/)**

A reader asked how to shell out to the operating system from C and keep the
logic testable. The Go version of this chapter uses `os/exec`; C uses `fork`,
`exec` and a pipe. It is more code, but it also makes very clear what "running
a command" actually does.

## Write the test first

We want a function that runs a program and captures its output:

```c
static void test_echo(void)
{
    char out[64];
    char *argv[] = {"echo", "hello", NULL};

    int n = exec_run(argv, out, sizeof out);

    CHECK_INT(n, 6);
    CHECK_STR(out, "hello\n");
}
```

## Write enough code to make it pass

There are four steps, and they are always the same four:

```c
int exec_run(char *const argv[], char *out, size_t cap)
{
    int fds[2];
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
    /* ... read loop into out ... */
    close(fds[0]);
    waitpid(pid, &status, 0);
    return (int)total;
}
```

- `pipe` creates two connected file descriptors
- `fork` makes a child
- in the child, `dup2` makes stdout *be* the pipe, then `execvp` **replaces**
  the child process with the program
- in the parent, we read the pipe and `waitpid` to reap the child

Two details are easy to get wrong and important:

- `execvp` only returns if it *failed*. If it fails, the child must `_exit`
  immediately — never let a failed `exec` fall through into the parent's code,
  which the child would then run a second time.
- The parent must `waitpid`, or the child becomes a zombie. Your OS reaper will
  eventually clean it up, but "eventually" is not a good look.

The parent reads the pipe *while the child runs*, so a command that produces
more output than the pipe buffer (often 64KB) cannot deadlock. Read-then-wait,
always.

## No shell, on purpose

`execvp` searches `PATH` and runs the program directly. It does **not** go
through `/bin/sh`. That matters:

```c
static void test_arguments_are_not_interpreted_by_a_shell(void)
{
    char out[64];
    char *argv[] = {"echo", "; rm -rf /", NULL};

    exec_run(argv, out, sizeof out);

    /* The argument is echoed literally; no shell ran the semicolon. */
    CHECK_STR(out, "; rm -rf /\n");
}
```

If you used `system()` or `popen()`, those arguments would reach a shell and a
semicolon would start a new command. Passing an `argv` array avoids the entire
class of shell-injection bugs. This is worth a test, because it is a security
property, not just behaviour.

## The testable seam

The trouble with `exec_run` is that testing your *business logic* by running
real programs is slow, environment-dependent and hard to control. The fix is
the same one from the [mocking](mocking.md) chapter: inject the runner.

```c
struct command_runner {
    int (*run)(void *ctx, char *const argv[], char *out, size_t cap);
    void *ctx;
};
```

The real runner wraps `exec_run`. A fake runner records the command and returns
canned output:

```c
static void test_current_user_trims_the_newline(void)
{
    struct fake_runner fake = {.output = "chris\n", .result = 6};
    struct command_runner runner = {.run = fake_run, .ctx = &fake};
    char name[64];

    CHECK_INT(current_user(runner, name, sizeof name), 0);
    CHECK_STR(name, "chris");
    CHECK_STR(fake.command, "whoami");
}
```

Now the logic — which command to run, how to trim its output, what to do when
it fails — is tested in microseconds, with no `whoami` on the machine at all.
The thin `exec_run` itself is tested separately with `echo`, which is
ubiquitous.

## Refactor

`exec_run` ignores the child's exit code and its standard error. A fuller
version would capture both and report the exit status, so "command not found"
and "command ran and failed" could be told apart. Our test for a missing
command checks only that no output appeared; the child's `_exit(127)` is
invisible to us.

Notice that the function has no notion of *why* the command failed, only that
it produced bytes or did not. That is a perfect segue into the next chapter,
on errors.

## Wrapping up

What we have covered:

- `pipe`, `fork`, `dup2`, `execvp`, `waitpid` — the four-step way to run a
  program
- Why the child must `_exit` when `exec` fails
- Why the parent reads before waiting, and why it must wait at all
- `execvp` versus a shell, and the injection bug you avoid
- Injecting the command runner so business logic is testable without processes

### Additional material

- [`fork` man page](https://man7.org/linux/man-pages/man2/fork.2.html)
- [`exec` man page](https://man7.org/linux/man-pages/man3/exec.3.html)
