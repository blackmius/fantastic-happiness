#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <unistd.h>

#define MAX_BUFFERS 10
//#define USE_SEM
char buf[MAX_BUFFERS][100];
int buffer_index;
int buffer_print_index;

sem_t mutex_sem, buffer_count_sem, spool_signal_sem;

void *producer(void *arg);
void *spooler(void *arg);

int main(int argc, char **argv) {
    pthread_t tid_producer [10], tid_spooler;
    buffer_index = buffer_print_index = 0;

    if (sem_init(&mutex_sem, 0, 1) == -1) {
        perror("sem_init"); exit(1);
    }
    
    if (sem_init(&buffer_count_sem, 0, MAX_BUFFERS) == -1) {
        perror("sem_init"); exit(1);
    }

    if (sem_init(&spool_signal_sem, 0, 0) == -1) {
        perror("sem_init"); exit(1);
    }

    int r;
    if ((r = pthread_create(&tid_spooler, NULL, spooler, NULL)) != 0) {
        fprintf(stderr, "Ошибка = %d (%s)\n", r, strerror(r)); exit(1);
    }

    int thread_no[10];
    for (int i = 0; i < 10; i++) {
        thread_no[i] = i;
        if ((r = pthread_create(&tid_producer[i], NULL, producer, (void *) &thread_no[i])) != 0) {
            fprintf(stderr, "Ошибка = %d (%s)\n", r, strerror(r)); exit(1);
        }
    }
    for (int i = 0; i < 10; i++) {
        if ((r = pthread_join(tid_producer[i], NULL)) == -1) {
            fprintf(stderr, "Ошибка = %d (%s)\n", r, strerror(r)); exit(1);
        }
    }
    
    int semval;
    while (1) {
        if (sem_getvalue(&spool_signal_sem, &semval) == -1)
            perror("sem_getvalue");
        if (!semval) break;
        sleep(1);
    }
    
    if ((r = pthread_cancel(tid_spooler)) != 0) {
        fprintf(stderr, "Ошибка = %d (%s)\n", r, strerror(r)); exit(1);
    }

    if (sem_destroy(&mutex_sem) == -1) {
        perror("sem_destroy"); exit(1);
    }

    if (sem_destroy(&mutex_sem) == -1) {
        perror("sem_destroy"); exit(1);
    }

    if (sem_destroy(&mutex_sem) == -1) {
        perror("sem_destroy"); exit(1);
    }
    exit (0);
}

void *producer (void *arg) {
    int my_id = *((int *) arg);
    int count = 0;

    for (int i = 0; i < 10; i++) {
        #if defined(USE_SEM)
        if (sem_wait (&buffer_count_sem) == -1) {
	        perror("sem_wait: buffer_count_sem"); exit(1);
        }
    
        
        if (sem_wait(&mutex_sem) == -1) {
	        perror("sem_wait: mutex_sem"); exit(1);
        }
        #endif

        int j = buffer_index;
        buffer_index++;
        if (buffer_index == MAX_BUFFERS)
            buffer_index = 0;

        #if defined(USE_SEM)
        if (sem_post(&mutex_sem) == -1) {
	        perror("sem_post: mutex_sem"); exit(1);
        }
        #endif
        
        sprintf(buf[j], "Поток %d: %d\n", my_id, ++count);
        if (sem_post(&spool_signal_sem) == -1) {
	        perror ("sem_post: spool_signal_sem"); exit(1);
        }
        
    
        sleep (1);
    }
}

void *spooler (void *arg) {
    while (1) {
        if (sem_wait(&spool_signal_sem) == -1) {
	        perror("sem_wait: spool_signal_sem"); exit(1);
        }
    
        printf("%s", buf[buffer_print_index]);
        
        buffer_print_index++;
        if (buffer_print_index == MAX_BUFFERS)
           buffer_print_index = 0;

        if (sem_post(&buffer_count_sem) == -1) {
	        perror ("sem_post: buffer_count_sem"); exit(1);
        }
    }
}
