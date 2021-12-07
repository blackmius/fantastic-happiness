#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

void show_help() {
    puts("Usage: ./map <pid: int> FILE?\n");
    exit(1);
}

int main(int argc, char *argv[]) {
    if (argc > 3 || argc < 2) show_help();
    int pid = atoi(argv[1]);
    if (pid == 0) {
        puts("Введенный pid не является целым числом\n");
        show_help();
    }
    char result[32] = {0x0};
    char cmd[16];
    sprintf(cmd, "pmap %d", pid);
    FILE *cmd_f = popen(cmd, "r");
    if (argc == 3) {
        char* fname = argv[2];
        while (access(fname, F_OK) == 0) {
            puts("Файл с указанным именем уже существует, он будет перезаписан.\nУказать другое имя файла? [Y/n]: ");
            char a = getchar();
            if (a == 'n') break;
            scanf("%s", fname);
        }
        FILE* f = fopen(fname, "w");
        if (f == NULL) {
            puts("Невозможно отрыть файл\n");
            return 1;
        }
        while (fgets(result, sizeof(result), cmd_f) != NULL)
           fputs(result, f);
    } else {
        while (fgets(result, sizeof(result), cmd_f) != NULL)
           fputs(result, stdout);
    }
    pclose(cmd_f);
    return 0;
}
