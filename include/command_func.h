
#ifndef COMMAND_FUNC_H
#define COMMAND_FUNC_H

#include <sys/types.h>

void execute_command(char **argv, int background);

int split_commands_by_pipe(char **argv, char ***commands);
int execute_pipe_loop(char ***cmd_argv, int cmd_count, int background);
void cleanup_and_exit(pid_t *pids, int count, int exit_code);

int handle_redirection(char **argv, int *argc);

#endif
