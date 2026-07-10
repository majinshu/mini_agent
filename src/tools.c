#include "tools.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

void tool_read_file(const char *p, int st, int ed, ToolOutput *o) {
    memset(o, 0, sizeof(*o));
    FILE *f = fopen(p, "r");
    if (!f) {
        o->result = errno==EACCES ? TOOL_PERMISSION_DENIED : TOOL_NOT_FOUND;
        snprintf(o->output, sizeof(o->output), "not found");
        return;
    }
    int tot = 0;
    char b[4096];
    while (fgets(b, sizeof(b), f)) tot++;
    rewind(f);
    if (st < 0 && ed < 0) {
        if (tot > 100) {
            fclose(f);
            o->result = TOOL_TOO_MANY_LINES;
            snprintf(o->output, sizeof(o->output),
                     "file has %d lines, max 100", tot);
            return;
        }
        st = 1; ed = tot;
    }
    if (ed > tot) ed = tot;
    if (st < 1) st = 1;
    int ln = ed - st + 1;
    if (ln > 500) {
        fclose(f);
        o->result = TOOL_TOO_MANY_LINES;
        snprintf(o->output, sizeof(o->output), "max 500 lines");
        return;
    }
    int cur = 0, pos = 0;
    while (fgets(b, sizeof(b), f)) {
        cur++;
        if (cur < st || cur > ed) continue;
        int r = (int)sizeof(o->output) - pos;
        if (r > 1) {
            int n = snprintf(o->output+pos, r, "%s", b);
            if (n > 0) pos += n;
        }
    }
    fclose(f);
    o->result = TOOL_OK;
}

void tool_write_file(const char *p, const char *c, ToolOutput *o) {
    memset(o, 0, sizeof(*o));
    FILE *f = fopen(p, "a");
    if (!f) {
        o->result = TOOL_PERMISSION_DENIED;
        return;
    }
    fprintf(f, "%s\n", c ? c : "");
    fclose(f);
    o->result = TOOL_OK;
    snprintf(o->output, sizeof(o->output), "write success");
}

void tool_exec_cmd(const char *c, ToolOutput *o) {
    memset(o, 0, sizeof(*o));
    int pfd[2];
    if (pipe(pfd) < 0) return;
    pid_t pid = fork();
    if (pid < 0) { close(pfd[0]); close(pfd[1]); return; }
    if (pid == 0) {
        close(pfd[0]);
        dup2(pfd[1], 1); dup2(pfd[1], 2);
        close(pfd[1]);
        execlp("/bin/sh", "sh", "-c", c, NULL);
        _exit(127);
    }
    close(pfd[1]);
    int pos = 0, to = 0;
    while (1) {
        fd_set f;
        struct timeval tv;
        FD_ZERO(&f); FD_SET(pfd[0], &f);
        tv.tv_sec = 10; tv.tv_usec = 0;
        int r = select(pfd[0]+1, &f, NULL, NULL, &tv);
        if (r <= 0) { if (r == 0) to = 1; break; }
        char b[4096];
        int n = read(pfd[0], b, sizeof(b)-1);
        if (n <= 0) break;
        b[n] = '\0';
        int rm = (int)sizeof(o->output) - pos;
        if (rm > 1) { int w = snprintf(o->output+pos, rm, "%s", b); if (w>0) pos+=w; }
    }
    close(pfd[0]);
    if (to) {
        kill(pid, SIGKILL);
        waitpid(pid, NULL, WNOHANG);
        o->timed_out = 1;
        o->result = TOOL_TIMEOUT;
    } else {
        int s;
        if (waitpid(pid, &s, 0) > 0)
            o->exit_code = WIFEXITED(s) ? WEXITSTATUS(s) : -1;
    }
}
