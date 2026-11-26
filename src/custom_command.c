#include "include.h"
#include "custom_command.h"

int custom_ls(char **argv)
{
    const char *path = argv[1] ? argv[1] : ".";
    DIR *dir = opendir(path);
    if (!dir) { perror(path); return 1; }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 &&
            strcmp(entry->d_name, "..") != 0)
            printf("%s\n", entry->d_name);
    }
    if (closedir(dir) != 0) { perror("closedir"); return 1; }
    return 0;
}

int custom_mkdir(char **argv)
{
    if (!argv[1]) { fprintf(stderr, "사용법: mkdir <디렉터리>\n"); return 1; }
    if (mkdir(argv[1], 0777) == 0) return 0;
    perror("mkdir"); return 1;
}

int custom_rmdir(char **argv)
{
    if (!argv[1]) { fprintf(stderr, "사용법: rmdir <디렉터리>\n"); return 1; }
    if (rmdir(argv[1]) == 0) return 0;
    perror("rmdir"); return 1;
}

int custom_ln(char **argv)
{
    if (!argv[1] || !argv[2]) {
        fprintf(stderr, "사용법: ln <원본> <링크>\n"); return 1;
    }
    if (link(argv[1], argv[2]) == 0) return 0;
    perror("ln"); return 1;
}

int custom_cp(char **argv)
{
    if (!argv[1] || !argv[2]) {
        fprintf(stderr, "사용법: cp <원본> <대상>\n"); return 1;
    }
    if (strcmp(argv[1], argv[2]) == 0) {
        fprintf(stderr, "cp: 원본과 대상이 같습니다.\n"); return 1;
    }

    FILE *in = fopen(argv[1], "rb");
    if (!in) { perror(argv[1]); return 1; }
    FILE *out = fopen(argv[2], "wb");
    if (!out) { perror(argv[2]); fclose(in); return 1; }

    char buffer[4096];
    size_t n;
    int failed = 0;
    while ((n = fread(buffer, 1, sizeof(buffer), in)) > 0) {
        if (fwrite(buffer, 1, n, out) != n) { perror("cp"); failed = 1; break; }
    }
    if (ferror(in)) { perror("cp: 읽기"); failed = 1; }
    if (fclose(in) != 0) { perror("cp: 입력 닫기"); failed = 1; }
    if (fclose(out) != 0) { perror("cp: 출력 닫기"); failed = 1; }
    return failed ? 1 : 0;
}

int custom_rm(char **argv)
{
    if (!argv[1]) { fprintf(stderr, "사용법: rm <파일>\n"); return 1; }
    if (unlink(argv[1]) == 0) return 0;
    perror("rm"); return 1;
}

int custom_mv(char **argv)
{
    if (!argv[1] || !argv[2]) {
        fprintf(stderr, "사용법: mv <원본> <대상>\n"); return 1;
    }
    if (rename(argv[1], argv[2]) == 0) return 0;
    perror("mv"); return 1;
}

int custom_cat(char **argv)
{
    FILE *fp = argv[1] ? fopen(argv[1], "r") : stdin;
    if (!fp) { perror("cat"); return 1; }

    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), fp))
        fputs(buffer, stdout);

    int failed = ferror(fp);
    if (argv[1] && fclose(fp) != 0) failed = 1;
    if (failed) { perror("cat"); return 1; }
    return 0;
}

int custom_grep(char **argv)
{
    if (!argv[1]) {
        fprintf(stderr, "사용법: grep <검색어> [파일]\n"); return 1;
    }
    FILE *fp = argv[2] ? fopen(argv[2], "r") : stdin;
    if (!fp) { perror("grep"); return 1; }

    char line[4096];
    int matched = 0;
    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, argv[1])) {
            fputs(line, stdout);
            matched = 1;
        }
    }
    int failed = ferror(fp);
    if (argv[2] && fclose(fp) != 0) failed = 1;
    if (failed) { perror("grep"); return 2; }
    return matched ? 0 : 1;
}

int custom_command(char **argv)
{
    if (!argv || !argv[0]) return 0;

    if (strcmp(argv[0], "ls") == 0) return custom_ls(argv);
    if (strcmp(argv[0], "mkdir") == 0) return custom_mkdir(argv);
    if (strcmp(argv[0], "rmdir") == 0) return custom_rmdir(argv);
    if (strcmp(argv[0], "ln") == 0) return custom_ln(argv);
    if (strcmp(argv[0], "cp") == 0) return custom_cp(argv);
    if (strcmp(argv[0], "rm") == 0) return custom_rm(argv);
    if (strcmp(argv[0], "mv") == 0) return custom_mv(argv);
    if (strcmp(argv[0], "cat") == 0) return custom_cat(argv);
    if (strcmp(argv[0], "grep") == 0) return custom_grep(argv);

    return -1;
}
