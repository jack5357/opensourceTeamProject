// ls 명령어 구현
int custom_ls(char **argv) {
    const char *path = argv[1] ? argv[1] : ".";
    struct dirent *entry;
    DIR *dir = opendir(path);
    if (dir == NULL) {
        perror(path);
        return 1;
    }

    // 디렉터리 내부 항목 탐색 및 출력
    while ((entry = readdir(dir))) {
        const char *name = entry->d_name;
        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
            continue;
        fprintf(stdout, "%s\n", name);
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
    const char *dirname = argv[1];
    if (dirname == NULL) {
        fprintf(stderr, "삭제할 디렉토리 이름을 지정해주세요\n");
        return 1;
    }
    if (rmdir(dirname) == 0)
        return 0;
    perror("rmdir");
    return 1;
}

// ln 명령어 구현
int custom_ln(char **argv) {
    const char *original = argv[1];
    const char *new_link = argv[2];
    if (original == NULL || new_link == NULL) {
        fprintf(stderr, "원본 파일과 링크 파일 이름을 확인해주세요\n");
        return 1;
    }

    // 하드링크 생성 결과 확인
    int result = link(original, new_link);
    if (result == 0) {
        return 0;
    }
    perror("ln");
    return 1;
}

// cp 명령어 구현
int custom_cp(char **argv) {
    const char *source = argv[1];
    const char *destination = argv[2];
    if (!source || !destination) {
        fprintf(stderr, "원본 파일과 복사할 파일의 이름을 확인해주세요\n");
        return 1;
    }

    FILE *input = fopen(source, "rb");
    if (!input) {
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
    // 원본 데이터를 읽어 대상 파일에 기록
    while ((size = fread(buffer, 1, sizeof(buffer), input)) != 0) {
        if (fwrite(buffer, 1, size, output) != size) {
            perror("cp");
            fclose(input);
            fclose(output);
            return 1;
        }
    }

    // 파일 읽기 오류 확인
    if (ferror(input)) {
        perror("cp");
        fclose(input);
        fclose(output);
        return 1;
    }

    fclose(input);
    fclose(output);
    return 0;
}

// rm 명령어 구현
int custom_rm(char **argv) {
    const char *target = argv[1];
    if (target == NULL) {
        fprintf(stderr, "삭제할 파일 이름을 입력하세요.\n");
        return 1;
    }

    // 지정된 파일 제거
    int result = unlink(target);
    if (result == 0) {
        return 0;
    }
    perror("rm");
    return 1;
}

// mv 명령어 구현
int custom_mv(char **argv) {
    if (!argv || !argv[1] || !argv[2]) {
        fprintf(stderr, "사용법: mv <원본파일> <새파일>\n");
        return EXIT_FAILURE;
    }

    const char *old_name = argv[1];
    const char *new_name = argv[2];

    // 파일 이름 변경 및 경로 이동
    int result = rename(old_name, new_name);
    if (result != 0) {
        perror("mv");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

// cat 명령어 구현
int custom_cat(char **argv) {
    FILE *fp;
    int use_stdin = (argv[1] == NULL);

    // 입력 대상 결정
    if (use_stdin) {
        fp = stdin;
    } else {
        fp = fopen(argv[1], "r");
        if (!fp) {
            perror("cat");
            return 1;
        }
    }

    char buffer[4096];
    // 입력 내용을 표준 출력으로 전달
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        fputs(buffer, stdout);
    }

    // 파일 입력인 경우에만 스트림 종료
    if (!use_stdin) {
        fclose(fp);
    }
    return 0;
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

    // 검색 대상 파일 확인
    if (argv[2] != NULL) {
        fp = fopen(argv[2], "r");
        if (fp == NULL) {
            perror("grep");
            return 1;
        }
        close_file = 1;
    }

    char line[4096];
    // 검색어가 포함된 행만 출력
    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, keyword)) {
            fputs(line, stdout);
        }
    }

    // 직접 열었던 파일만 닫기
    if (close_file)
        fclose(fp);
    return 0;
}
