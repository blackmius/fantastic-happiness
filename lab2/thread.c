#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

void* thread(void* ptr) {
    pthread_t parent;
    parent = *(pthread_t *) ptr;
    time_t rawtime;
    struct tm * timeinfo;
    time( &rawtime );
    timeinfo = localtime( &rawtime );
    printf(
        "Это Дочерний поток его id=%lu, а его родительский поток id=%lu, текущее время %02d:%02d:%02d\n",
        pthread_self(),
        parent,
        timeinfo->tm_hour,
        timeinfo->tm_min,
        timeinfo->tm_sec
    );
}

int main () {
    pthread_t parent, thread1, thread2;
    parent = pthread_self();
    printf("Главный поток id=%lu\n", parent);
    pthread_create(&thread1, NULL, &thread, &parent);
    pthread_create(&thread2, NULL, &thread, &parent);
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL); 
}
