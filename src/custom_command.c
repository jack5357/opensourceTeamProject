#include "include.h"
#include "custom_command.h"

/* 기존 원문의 custom_ls()부터 custom_grep()까지 그대로 유지 */

int custom_command(char **argv)
{
    if (!argv || !argv[0])
        return 0;

    if (strcmp(argv[0], "ls") == 0)
        return custom_ls(argv);
    if (strcmp(argv[0], "mkdir") == 0)
        return custom_mkdir(argv);
    if (strcmp(argv[0], "rmdir") == 0)
        return custom_rmdir(argv);
    if (strcmp(argv[0], "ln") == 0)
        return custom_ln(argv);
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

    return -1;
}
