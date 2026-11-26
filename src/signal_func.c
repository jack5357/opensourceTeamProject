#include "include.h"
#include "signal_func.h"

void setup_shell_signals(void)
{
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
}

void setup_child_signals(void)
{
    signal(SIGINT, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGTSTP, SIG_DFL);
    signal(SIGPIPE, SIG_DFL);
}

void reap_background_processes(void)
{
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        printf("[완료] 백그라운드 프로세스 %d\n", (int)pid);
    }
}
