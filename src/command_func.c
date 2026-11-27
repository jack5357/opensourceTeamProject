
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
            fprintf(stderr, "리다이렉션 파일 이름이 없습니다.\n");
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
