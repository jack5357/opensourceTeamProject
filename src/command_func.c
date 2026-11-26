#include "include.h"
#include "config.h"
#include "command_func.h"
#include "custom_command.h"
#include "signal_func.h"

int handle_redirection(char **argv)
{
    int in_fd = -1, out_fd = -1;
    int read_index = 0, write_index = 0;

    while (argv[read_index] != NULL) {
        char *token = argv[read_index];

        if (strcmp(token, "<") == 0 ||
            strcmp(token, ">") == 0 ||
            strcmp(token, ">>") == 0) {
            int is_input = (strcmp(token, "<") == 0);
            int append = (strcmp(token, ">>") == 0);
            char *filename = argv[read_index + 1];

            if (filename == NULL || filename[0] == '\0') {
                fprintf(stderr, "myshell: 리다이렉션 파일명이 없습니다.\n");
                if (in_fd >= 0) close(in_fd);
                if (out_fd >= 0) close(out_fd);
                return -1;
            }

            int fd;
            if (is_input)
                fd = open(filename, O_RDONLY);
            else
                fd = open(filename, O_WRONLY | O_CREAT |
                          (append ? O_APPEND : O_TRUNC), 0666);

            if (fd < 0) {
                perror(filename);
                if (in_fd >= 0) close(in_fd);
                if (out_fd >= 0) close(out_fd);
                return -1;
            }

            if (is_input) {
                if (in_fd >= 0) close(in_fd);
                in_fd = fd;
            } else {
                if (out_fd >= 0) close(out_fd);
                out_fd = fd;
            }
            read_index += 2;
        } else {
            argv[write_index++] = argv[read_index++];
        }
    }
    argv[write_index] = NULL;

    if (in_fd >= 0) {
        if (dup2(in_fd, STDIN_FILENO) < 0) {
            perror("dup2");
            close(in_fd);
            if (out_fd >= 0) close(out_fd);
            return -1;
        }
        close(in_fd);
    }
    if (out_fd >= 0) {
        if (dup2(out_fd, STDOUT_FILENO) < 0) {
            perror("dup2");
            close(out_fd);
            return -1;
        }
        close(out_fd);
    }
    return 0;
}

int split_commands_by_pipe(char **argv, char ***commands)
{
    int count = 0;
    if (argv == NULL || argv[0] == NULL)
        return -1;

    commands[count++] = argv;
    for (int i = 0; argv[i] != NULL; ++i) {
        if (strcmp(argv[i], "|") != 0)
            continue;

        if (i == 0 || argv[i + 1] == NULL ||
            strcmp(argv[i + 1], "|") == 0 || count > MAX_PIPES) {
            fprintf(stderr, "myshell: 잘못된 파이프 명령입니다.\n");
            return -1;
        }
        argv[i] = NULL;
        commands[count++] = &argv[i + 1];
    }
    return count;
}

static int run_one_command(char **argv)
{
    if (argv[0] == NULL)
        return 0;

    if (handle_redirection(argv) < 0)
        return -1;

    int result = custom_command(argv);
    if (result == -1) {
        execvp(argv[0], argv);
        perror(argv[0]);
        return -1;
    }
    return result;
}

int execute_pipe_loop(char ***commands, int command_count, int background)
{
    int previous_read = STDIN_FILENO;
    pid_t pids[MAX_PIPES + 1];
    int started = 0;

    for (int i = 0; i < command_count; ++i) {
        int pipe_fd[2] = {-1, -1};
        int has_next = (i < command_count - 1);

        if (has_next && pipe(pipe_fd) < 0) {
            perror("pipe");
            if (previous_read != STDIN_FILENO) close(previous_read);
            return -1;
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            if (previous_read != STDIN_FILENO) close(previous_read);
            if (pipe_fd[0] >= 0) close(pipe_fd[0]);
            if (pipe_fd[1] >= 0) close(pipe_fd[1]);
            for (int j = 0; j < started; ++j) kill(pids[j], SIGTERM);
            for (int j = 0; j < started; ++j) waitpid(pids[j], NULL, 0);
            return -1;
        }

        if (pid == 0) {
            setup_child_signals();

            if (previous_read != STDIN_FILENO) {
                if (dup2(previous_read, STDIN_FILENO) < 0) {
                    perror("dup2");
                    _exit(EXIT_FAILURE);
                }
                close(previous_read);
            }
            if (has_next) {
                close(pipe_fd[0]);
                if (dup2(pipe_fd[1], STDOUT_FILENO) < 0) {
                    perror("dup2");
                    _exit(EXIT_FAILURE);
                }
                close(pipe_fd[1]);
            }

            int result = run_one_command(commands[i]);
            fflush(NULL);
            _exit(result == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
        }

        pids[started++] = pid;
        if (previous_read != STDIN_FILENO)
            close(previous_read);
        if (has_next) {
            close(pipe_fd[1]);
            previous_read = pipe_fd[0];
        }
    }

    if (background) {
        printf("[%d] 파이프라인 (백그라운드 실행)\n", (int)pids[0]);
    } else {
        for (int i = 0; i < started; ++i) {
            int status;
            while (waitpid(pids[i], &status, 0) < 0 && errno == EINTR) {}
        }
    }
    return 0;
}

int execute_command(char **argv, int background)
{
    char **commands[MAX_PIPES + 1];
    int count = split_commands_by_pipe(argv, commands);
    if (count < 0)
        return -1;

    if (count > 1)
        return execute_pipe_loop(commands, count, background);

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return -1;
    }

    if (pid == 0) {
        setup_child_signals();
        int result = run_one_command(argv);
        fflush(NULL);
        _exit(result == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
    }

    if (background) {
        printf("[%d] %s (백그라운드 실행)\n", (int)pid, argv[0]);
    } else {
        int status;
        while (waitpid(pid, &status, 0) < 0) {
            if (errno == EINTR) continue;
            perror("waitpid");
            break;
        }
    }
    return 0;
}
