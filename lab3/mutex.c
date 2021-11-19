#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>

//#define USE_MUTEX
#define THREADS 10

pthread_t tid[THREADS];

FILE *fp;
pthread_mutex_t lock;

void* thread(void *arg) {
    #if defined(USE_MUTEX)
        pthread_mutex_lock(&lock);
    #endif
    sleep(rand() % 3);
    for (int i = 0; i < 10; i++) {
        fputs("Запись ", fp);
        fputs("из ", fp);
        fputs("потока ", fp);
        fprintf(fp, "%lu ", pthread_self());
        fprintf(fp, "Номер %d\n", i);
    }
    #if defined(USE_MUTEX)
        pthread_mutex_unlock(&lock);
    #endif
    
    return NULL;
}

int main(void) {
    fp = fopen("./mutex.txt", "w");
    for (int i = 0; i < THREADS; i++) {
        int err = pthread_create(&(tid[i]), NULL, &thread, NULL);
        if (err != 0) {
            printf("Невозможно создать поток :[%s]\n", strerror(err));
        } else {
            printf("Создан поток %d\n", i);
        }
    }
    for (int i = 0; i < THREADS; i++) {
        pthread_join(tid[i], NULL);
    }
    fclose(fp);
    return 0;
}
