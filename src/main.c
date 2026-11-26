#include "include.h"
#include "config.h"
#include "command_func.h"
#include "signal_func.h"

int main(void)
{
    char line[SHELL_INPUT_SIZE];
    char *argv[MAX_ARGS];

    setup_shell_signals();

    while (1) {
        reap_background_processes();

        printf("myshell> ");
        fflush(stdout);

        if (fgets(line, sizeof(line), stdin) == NULL)
            break;

        int argc = 0;
        char *token = strtok(line, " \t\n");

        while (token != NULL && argc < MAX_ARGS - 1) {
            argv[argc++] = token;
            token = strtok(NULL, " \t\n");
        }
        argv[argc] = NULL;

        if (argc == 0)
            continue;

        // exit 및 cd 명령어 처리
        if (strcmp(argv[0], "exit") == 0) {
            printf("쉘을 종료합니다.\n");
            break;
        }
        else if (strcmp(argv[0], "cd") == 0) {
            char *target_path = getenv("HOME");

            if (argc > 1)
                target_path = argv[1];

            if (target_path != NULL && chdir(target_path) != 0)
                perror("chdir");

            continue;
        }

        // 백그라운드 명령어 판별
        int run_background = 0;

        if (argc > 0 && !strcmp(argv[argc - 1], "&")) {
            run_background = 1;
            argv[--argc] = NULL;
        }

        // 명령어 실행 처리
        if (argc == 0)
            continue;

        execute_command(argv, run_background);
    }

    return 0;
}
