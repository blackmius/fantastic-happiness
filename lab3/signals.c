#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

void sig_handler_parent(int signum) {
  printf("Родитель получил сигнал от ребенка\n");
}

void sig_handler_child(int signum) {
  printf("Ребенок получил сигнал от родителя\n");
  sleep(1);
  kill(getppid(), SIGUSR1);
}

int main(){
  pid_t pid;
  if ((pid=fork()) < 0) {
    printf("Ошибка при создании дочернего процесса\n");
    exit(1);
  } else if (pid == 0) {
    signal(SIGUSR1, sig_handler_child);
    printf("Ребенок ожидает сигнал\n");
    pause();
  } else {
    signal(SIGUSR1, sig_handler_parent);
    sleep(1);
    printf("Родитель отправляет сигнал ребенку\n");
    kill(pid, SIGUSR1);
    printf("Родитель ожидает сигнал в ответ\n");
    pause();
  }
  return 0;
}
