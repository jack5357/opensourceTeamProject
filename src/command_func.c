
#include "include.h"
#include "config.h"
#include "command_func.h"
#include "custom_command.h"
#include "signal_func.h"

// 명령어 실행 및 프로세스 관리
void execute_command(char **argv, int background)
{
    char **cmd_argv[MAX_PIPES + 1];
    int cmd_count = split_commands_by_pipe(argv, cmd_argv);

    if (cmd_count == -1)
        return;

    // 파이프 명령어 실행
    if (cmd_count > 1) {
        execute_pipe_loop(cmd_argv, cmd_count, background);
        return;
    }

    pid_t pid = fork();
    int status;

    if (pid < 0) {
        perror("fork");
        return;
    }

    // 자식 프로세스 실행
    if (pid == 0) {
        setup_child_signals();

        int argc_dummy = 0;
        if (handle_redirection(argv, &argc_dummy) == -1)
            exit(EXIT_FAILURE);

        // 명령어 처리
        if (custom_command(argv) == -1) {
            execvp(argv[0], argv);
            perror(argv[0]);
            exit(EXIT_FAILURE);
        }

        exit(EXIT_SUCCESS);
    }

    // 부모 프로세스 실행
    if (background == 1) {
        printf("[%d] %s (백그라운드 실행)\n", pid, argv[0]);
    } else {
        pid_t result;

        do {
            result = waitpid(pid, &status, WUNTRACED);
        } while (result == -1 && errno == EINTR);
    }
}

// 파이프라인 명령어 실행
int execute_pipe_loop(char ***cmd_argv, int cmd_count, int background)
{
    int pipe_fds[2];
    int input_fd = STDIN_FILENO;
    pid_t pids[MAX_PIPES + 1];
    int current_pids_count = 0;

    for (int i = 0; i < cmd_count; i++) {
        char **current_cmd = cmd_argv[i];
        int has_next = (i < cmd_count - 1);

        // 다음 명령어가 있을 때 파이프 생성
        if (has_next && pipe(pipe_fds) == -1) {
            perror("pipe");

            if (input_fd != STDIN_FILENO)
                close(input_fd);

            cleanup_and_exit(pids, current_pids_count, EXIT_FAILURE);
            return -1;
        }

        pid_t pid = fork();

        if (pid < 0) {
            perror("fork");

            if (input_fd != STDIN_FILENO)
                close(input_fd);

            if (has_next) {
                close(pipe_fds[0]);
                close(pipe_fds[1]);
            }

            cleanup_and_exit(pids, current_pids_count, EXIT_FAILURE);
            return -1;
        }

        // 자식 프로세스
        if (pid == 0) {
            setup_child_signals();

            if (input_fd != STDIN_FILENO) {
                dup2(input_fd, STDIN_FILENO);
                close(input_fd);
            }

            if (has_next) {
                dup2(pipe_fds[1], STDOUT_FILENO);
                close(pipe_fds[0]);
                close(pipe_fds[1]);
            }

            int argc_dummy = 0;
            if (handle_redirection(current_cmd, &argc_dummy) == -1)
                exit(EXIT_FAILURE);

            // 현재 명령어 실행
            if (custom_command(current_cmd) == -1) {
                execvp(current_cmd[0], current_cmd);
                perror(current_cmd[0]);
                exit(EXIT_FAILURE);
            }

            exit(EXIT_SUCCESS);
        }

        // 부모 프로세스
        pids[current_pids_count++] = pid;

        if (input_fd != STDIN_FILENO)
            close(input_fd);

        if (has_next) {
            input_fd = pipe_fds[0];
            close(pipe_fds[1]);
        }
    }

    // 포그라운드 실행 시 자식 프로세스 대기
    if (background == 0) {
        for (int i = 0; i < current_pids_count; i++) {
            int status;
            waitpid(pids[i], &status, 0);
        }
    } else {
        printf("[%d] 파이프라인 (백그라운드 실행)\n", pids[0]);
    }

    return 0;
}

// 파이프 연산자를 기준으로 명령어 분리
int split_commands_by_pipe(char **argv, char ***commands)
{
    int cmd_count = 0;
    int argc = 0;

    if (argv == NULL || argv[0] == NULL)
        return -1;

    while (argv[argc] != NULL)
        argc++;

    commands[cmd_count++] = argv;

    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "|") != 0)
            continue;

        if (i == 0 || i + 1 >= argc ||
            strcmp(argv[i + 1], "|") == 0 ||
            cmd_count >= MAX_PIPES + 1) {
            fprintf(stderr, "myshell: 잘못된 파이프 명령입니다.\n");
            return -1;
        }

        argv[i] = NULL;
        commands[cmd_count++] = &argv[i + 1];
    }

    return cmd_count;
}

// 파이프라인 오류 발생 시 자식 프로세스 정리
void cleanup_and_exit(pid_t *pids, int count, int exit_code)
{
    fprintf(stderr, "myshell: 파이프라인 오류 발생. 자식 프로세스 정리 중...\n");

    for (int i = 0; i < count; i++) {
        if (pids[i] > 0)
            kill(pids[i], SIGTERM);
    }

    int status;

    for (int i = 0; i < count; i++) {
        if (pids[i] > 0) {
            while (waitpid(pids[i], &status, 0) == -1 && errno == EINTR)
                continue;
        }
    }

    exit(exit_code);
}

// 입출력 리다이렉션 처리
int handle_redirection(char **argv, int *argc)
{
    if (argv == NULL)
        return -1;

    int count = 0;

    while (argv[count] != NULL)
        count++;

    int i = 0;

    while (i < count) {
        int target_fd = -1;
        int flags = 0;

        if (strcmp(argv[i], "<") == 0) {
            target_fd = STDIN_FILENO;
            flags = O_RDONLY;
        }
        else if (strcmp(argv[i], ">") == 0) {
            target_fd = STDOUT_FILENO;
            flags = O_WRONLY | O_CREAT | O_TRUNC;
        }
        else if (strcmp(argv[i], ">>") == 0) {
            target_fd = STDOUT_FILENO;
            flags = O_WRONLY | O_CREAT | O_APPEND;
        }
        else {
            i++;
            continue;
        }

        if (i + 1 >= count) {
            fprintf(stderr, "myshell: 리다이렉션 파일 이름이 없습니다.\n");
            return -1;
        }

        const char *filename = argv[i + 1];
        int fd;

        if (target_fd == STDIN_FILENO)
            fd = open(filename, flags);
        else
            fd = open(filename, flags, 0666);

        if (fd == -1) {
            perror(filename);
            return -1;
        }

        if (dup2(fd, target_fd) == -1) {
            perror("dup2");
            close(fd);
            return -1;
        }

        close(fd);

        // 리다이렉션 연산자와 파일 이름 제거
        for (int j = i; j + 2 <= count; j++)
            argv[j] = argv[j + 2];

        count -= 2;
    }

    if (argc != NULL)
        *argc = count;

    return 0;
}
