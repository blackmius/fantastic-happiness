#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>

int main () {
    pid_t pid1, pid2;
    (pid1 = fork()) && (pid2 = fork());
    if (pid1 == 0 || pid2 == 0) {
        time_t rawtime;
        struct tm * timeinfo;
        time( &rawtime );
        timeinfo = localtime( &rawtime );
        printf(
            "Это Дочерний процесс его pid=%d, а его родительский процесс pid=%d, текущее время %02d:%02d:%02d\n",
            getpid(),
            getppid(),
            timeinfo->tm_hour,
            timeinfo->tm_min,
            timeinfo->tm_sec
        );
    } else if (pid1 > 0 && pid2 > 0) {
        printf("Это Родительский процесс pid=%d\n", getpid());
        char cmd[32];
        sprintf(cmd, "ps -x | grep '^  %d\\|^  %d'", pid1, pid2);
        system(cmd);
        while(wait(NULL) > 0);
    } else {
        printf("Ошибка вызова fork, потомок не создан\n");
    }
}
