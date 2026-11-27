
#include "include.h"
#include "custom_command.h"

// ls 명령어 구현
int custom_ls(char **argv) {
    const char *path = (argv[1] != NULL) ? argv[1] : ".";
    DIR *dir = opendir(path);
    struct dirent *entry;

    if (dir == NULL) {
        perror(path);
        return 1;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 &&
            strcmp(entry->d_name, "..") != 0) {
            fprintf(stdout, "%s\n", entry->d_name);
        }
    }

    closedir(dir);
    return 0;
}

// mkdir 명령어 구현
int custom_mkdir(char **argv) {
    const char *dirname = argv[1];

    if (dirname == NULL) {
        fprintf(stderr, "생성할 디렉토리 이름을 지정해주세요\n");
        return 1;
    }

    if (mkdir(dirname, 0777) == 0)
        return 0;

    perror("mkdir");
    return 1;
}

// rmdir 명령어 구현
int custom_rmdir(char **argv) {
    const char *target = argv[1];

    if (target == NULL) {
        fprintf(stderr, "삭제할 디렉토리 이름을 지정해주세요\n");
        return 1;
    }

    if (rmdir(target) == 0)
        return 0;

    perror("rmdir");
    return 1;
}

// cp 명령어 구현
int custom_cp(char **argv) {
    const char *source = argv[1];
    const char *destination = argv[2];

    if (source == NULL || destination == NULL) {
        fprintf(stderr, "원본 파일과 복사할 파일의 이름을 확인해주세요\n");
        return 1;
    }
    struct stat source_stat;
    struct stat destination_stat;

    if (stat(source, &source_stat) == 0 &&
        stat(destination, &destination_stat) == 0) {

        if (source_stat.st_dev == destination_stat.st_dev &&
            source_stat.st_ino == destination_stat.st_ino) {
            fprintf(stderr, "원본과 복사 대상이 같은 파일입니다.\n");
            return 1;
        }
}

    FILE *input = fopen(source, "rb");
    if (input == NULL) {
        perror("원본");
        return 1;
    }

    FILE *output = fopen(destination, "wb");
    if (output == NULL) {
        perror("복사본");
        fclose(input);
        return 1;
    }

    char buffer[4096];
    size_t size;
    int result = 0;

    while ((size = fread(buffer, 1, sizeof(buffer), input)) > 0) {
        if (fwrite(buffer, 1, size, output) != size) {
            perror("cp");
            result = 1;
            break;
        }
    }

    if (ferror(input)) {
        perror("cp");
        result = 1;
    }

    if (fclose(input) != 0) {
    perror("cp");
    result = 1;
    }

    if (fclose(output) != 0) {
    perror("cp");
    result = 1;
    }

    return result;
}

// rm 명령어 구현
int custom_rm(char **argv) {
    const char *target = argv[1];

    if (target == NULL) {
        fprintf(stderr, "삭제할 파일 이름을 입력하세요.\n");
        return 1;
    }

    if (unlink(target) == 0)
        return 0;

    perror("rm");
    return 1;
}

// mv 명령어 구현
int custom_mv(char **argv) {
    if (argv == NULL || argv[1] == NULL || argv[2] == NULL) {
        fprintf(stderr, "사용법: mv <원본파일> <새파일>\n");
        return 1;
    }

    if (rename(argv[1], argv[2]) == 0)
        return 0;

    perror("mv");
    return 1;
}

// cat 명령어 구현
int custom_cat(char **argv) {
    int use_stdin = (argv[1] == NULL);
    FILE *fp = use_stdin ? stdin : fopen(argv[1], "r");

    if (fp == NULL) {
        perror("cat");
        return 1;
    }

    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), fp) != NULL)
        fputs(buffer, stdout);

    int result = ferror(fp) ? 1 : 0;

    if (!use_stdin)
        fclose(fp);

    return result;
}

// grep 명령어 구현
int custom_grep(char **argv) {
    if (argv[1] == NULL) {
        fprintf(stderr, "사용법: grep <검색어> [파일]\n");
        return 1;
    }

    const char *keyword = argv[1];
    FILE *fp = stdin;
    int close_file = 0;

    if (argv[2] != NULL) {
        fp = fopen(argv[2], "r");
        if (fp == NULL) {
            perror("grep");
            return 1;
        }
        close_file = 1;
    }

    char line[4096];
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (strstr(line, keyword) != NULL)
            fputs(line, stdout);
    }

    int result = ferror(fp) ? 1 : 0;

    if (close_file)
        fclose(fp);

    return result;
}

// ln 명령어 구현 (하드 링크)
int custom_ln(char **argv) {
    const char *source = argv[1];
    const char *linkname = argv[2];

    if (source == NULL || linkname == NULL) {
        fprintf(stderr, "원본 파일과 링크 파일 이름을 확인해주세요\n");
        return 1;
    }

    if (link(source, linkname) == 0)
        return 0;

    perror("ln");
    return 1;
}

// pwd 명령어 구현
int custom_pwd(char **argv) {
    (void)argv;

    char cwd[PATH_MAX];

    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("pwd");
        return 1;
    }

    printf("%s\n", cwd);
    return 0;
}

// 사용자 정의 명령어 분기
int custom_command(char **argv) {
    if (argv == NULL || argv[0] == NULL)
        return 0;

    if (strcmp(argv[0], "ls") == 0)
        return custom_ls(argv);
    if (strcmp(argv[0], "mkdir") == 0)
        return custom_mkdir(argv);
    if (strcmp(argv[0], "rmdir") == 0)
        return custom_rmdir(argv);
    if (strcmp(argv[0], "cp") == 0)
        return custom_cp(argv);
    if (strcmp(argv[0], "rm") == 0)
        return custom_rm(argv);
    if (strcmp(argv[0], "mv") == 0)
        return custom_mv(argv);
    if (strcmp(argv[0], "cat") == 0)
        return custom_cat(argv);
    if (strcmp(argv[0], "grep") == 0)
        return custom_grep(argv);
    if (strcmp(argv[0], "ln") == 0)
        return custom_ln(argv);
    if (strcmp(argv[0], "pwd") == 0)
        return custom_pwd(argv);

    return -1;
}
