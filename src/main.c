// exit 및 cd 명령어 처리
if (strcmp(argv[0], "exit") == 0) {
    printf("쉘을 종료합니다.\n");
    return 0;
}
else if (strcmp(argv[0], "cd") == 0) {
    char *target_path = getenv("HOME");
    if (argc > 1) {
        target_path = argv[1];
    }
    if (chdir(target_path) != 0) {
        perror("chdir");
    }
    return 1;
}

// 백그라운드 명령어 판별
int run_background = 0;
if (argc > 0 && !strcmp(argv[argc - 1], "&")) {
    run_background = 1;
    argv[--argc] = NULL;
}

// 명령어 실행 처리
if (argc == 0) {
    return 1;
}
execute_command(argv, run_background);
return 1;
