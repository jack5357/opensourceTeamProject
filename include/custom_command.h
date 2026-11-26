#ifndef COMMAND_FUNC_H
#define COMMAND_FUNC_H

#include "include.h"

int execute_command(char **argv, int background);
int split_commands_by_pipe(char **argv, char ***commands);
int execute_pipe_loop(char ***commands, int command_count, int background);
int handle_redirection(char **argv);

#endif
