#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <string.h>

#define SEM_READ "/sem-read"
#define SEM_WRITE "/sem-write"
#define SHM "n.shm"

sem_t *read_sem, *write_sem;

void show(double **matrix, int n, int m) {
    for (int row = 0; row < n; row++) {
        for(int col = 0; col < m; col++)
            printf("%.2f\t", matrix[row][col]);
        printf("\n");
    }
}

void show_from(double **matrix, int n, int m, int row, int col) {
    for (int i = row; i < n; i++) {
        for(int j = col; j < m; j++)
            printf("%.2f\t", matrix[i][j]);
        printf("\n");
    }
}

int simplify(double **M, int n, int r) {
    if (M[r][r] == 0) {
        int row;
        for (row = 0; row < n-1; row++) {
            if (M[row][r] != 0) break;
        }
        if (M[row][r] == 0) {
            printf("Вырожденная матрица detA = 0\n");
            return 1;
        }
        double* c = M[row];
        M[row] = M[r];
        M[r] = c; 
    }
    for (int row = 0; row < n; row++) {
        if (row == r) {
            double f = M[r][r];
            for (int col = r; col < 2*n; col++) {
                M[row][col] /= f;
            }
        } else {
            double f = M[row][r] / M[r][r];
            for (int col = r; col < 2*n; col++) {
                M[row][col] -= M[r][col] * f;
            }
        }
    }
    return 0;
}

int main(void) {
    srand(time(NULL));
    int n = 2 + rand() % 5;
    
    sem_unlink(SEM_READ);
    sem_unlink(SEM_WRITE);
    if ((read_sem = sem_open(SEM_READ, O_CREAT, 0660, 0)) == SEM_FAILED) {
        perror ("sem_open"); exit (1);
    };
    if ((write_sem = sem_open(SEM_WRITE, O_CREAT, 0660, 0)) == SEM_FAILED) {
        perror ("sem_open"); exit (1);
    };
    
    double** A = (double**) malloc(n * sizeof(double*));
    for (int row = 0; row < n; row++) {
        A[row] = (double*) malloc(n * sizeof(double));
        for (int col = 0; col < n; col++)
            A[row][col] = rand() % 100;
    }
    printf("Сгенерированная матрица\n");
    show(A, n, n);
    
    
    double **B = (double**) malloc(n * sizeof(double*));
    for (int row = 0; row < n; row++) {
        B[row] = (double*) malloc(2 * n * sizeof(double));
        int col;
        for (col = 0; col < n; col++) B[row][col] = A[row][col];
    }
    for (int i = 0; i < n; i++) B[i][n+i] = 1;
    printf("Дополненная матрица\n");
    show(B, n, 2*n);
    
    int fd_shm;
    if ((fd_shm = shm_open(SHM, O_RDWR | O_CREAT, 0777)) < 0) {
        perror("error create shm"); return 1;
    }
    ftruncate(fd_shm, 2*sizeof(int)+2*n*sizeof(double));
    
    for (int i = 0; i < n; i++) {
        if (fork() == 0) {
            void* ptr = mmap(0, sizeof(int), PROT_WRITE, MAP_SHARED, fd_shm, 0);
            printf("Вычисление строки %d процессом номер %d\n", i, getpid());
            sem_wait(read_sem);
            n = *(int*) ptr;
            ptr = mmap(0, sizeof(int)+2*n*sizeof(double), PROT_WRITE, MAP_SHARED, fd_shm, 0);
            for (int j = 0; j < n; j++) memcpy(B[j], ptr + sizeof(int)+j*2*n*sizeof(double), 2*n*sizeof(double));
            *(int*) (ptr + sizeof(int)+2*n*sizeof(double)) = simplify(B, n, i);
            for (int j = 0; j < n; j++) memcpy(ptr + sizeof(int)+j*2*n*sizeof(double), B[j], 2*n*sizeof(double));
            sem_post(write_sem);
            return 0;
        }
    }
    int res = 0, i = 0;
    void* ptr = mmap(0, 2*sizeof(int)+2*n*sizeof(double), PROT_WRITE, MAP_SHARED, fd_shm, 0);
    *(int*)ptr = n;
    for (int j = 0; j < n; j++) memcpy(ptr + sizeof(int)+j*2*n*sizeof(double), B[j], 2*n*sizeof(double));
    while(i++ < n && !res) {
        sem_post(read_sem);
        res = *(int*) (ptr + sizeof(int)+2*n*sizeof(double));
        sem_wait(write_sem);
    }
    for (int j = 0; j < n; j++) memcpy(B[j], ptr + sizeof(int)+j*2*n*sizeof(double), 2*n*sizeof(double));
    if (!res) {
        printf("Обратная матрица\n");
        show_from(B, n, 2*n, 0, n);    
    }
    for(int i=0; i<n; i++) wait(NULL);
    
    
    shm_unlink(SHM);
    sem_unlink(SEM_READ);
    sem_unlink(SEM_WRITE);
}
