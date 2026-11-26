#include "include.h"
#include "config.h"
#include "command_func.h"
#include "signal_func.h"

static int parse_line(char *line, char **argv)
{
    int argc = 0;
    char *saveptr = NULL;
    char *token = strtok_r(line, " \t\r\n", &saveptr);

    while (token != NULL) {
        if (argc >= MAX_ARGS - 1) {
            fprintf(stderr, "myshell: 인자가 너무 많습니다.\n");
            return -1;
        }
        argv[argc++] = token;
        token = strtok_r(NULL, " \t\r\n", &saveptr);
    }
    argv[argc] = NULL;
    return argc;
}

int main(void)
{
    char line[SHELL_INPUT_SIZE];
    char *argv[MAX_ARGS];

    setup_shell_signals();

    while (1) {
        reap_background_processes();

        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) != NULL)
            printf("%s$ ", cwd);
        else
            printf("myshell$ ");
        fflush(stdout);

        if (fgets(line, sizeof(line), stdin) == NULL) {
            if (feof(stdin)) {
                putchar('\n');
                break;
            }
            if (errno == EINTR) {
                clearerr(stdin);
                putchar('\n');
                continue;
            }
            perror("fgets");
            continue;
        }

        int argc = parse_line(line, argv);
        if (argc <= 0)
            continue;

        if (strcmp(argv[0], "exit") == 0) {
            printf("쉘을 종료합니다.\n");
            break;
        }

        if (strcmp(argv[0], "cd") == 0) {
            const char *target = (argc > 1) ? argv[1] : getenv("HOME");
            if (target == NULL) {
                fprintf(stderr, "myshell: HOME 환경변수가 설정되지 않았습니다.\n");
            } else if (chdir(target) != 0) {
                perror("cd");
            }
            continue;
        }

        int background = 0;
        if (argc > 0 && strcmp(argv[argc - 1], "&") == 0) {
            background = 1;
            argv[--argc] = NULL;
        }
        if (argc == 0)
            continue;

        execute_command(argv, background);
    }

    return EXIT_SUCCESS;
}
