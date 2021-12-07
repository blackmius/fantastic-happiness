#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <semaphore.h>
#include <stddef.h>

#define SEM_READ "/sem-read"
#define SEM_WRITE "/sem-write"
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
    
    int sock = socket(AF_UNIX, SOCK_DGRAM, 0);
    struct sockaddr_un name; 
    name.sun_family = AF_UNIX;
    
    size_t size = (offsetof (struct sockaddr_un, sun_path) + strlen (name.sun_path));
    bind(sock, (struct sockaddr *) &name, size);
    
    for (int i = 0; i < n; i++) {
        if (fork() == 0) {
            printf("Вычисление строки %d процессом номер %d\n", i, getpid());
            //bind(sock, (struct sockaddr *) &name, size);
            sem_wait(read_sem);
            printf("Считал %lu байта n=%d\n", recvfrom(sock, &n, sizeof(int), 0, NULL, NULL), n);
            for (int j = 0; j < n; j++) {
                printf("Считал %lu байта получил строку ", recvfrom(sock, B[j], 2*n*sizeof(double), 0, NULL, NULL));
                int k;
                for (k = 0; k < 2*n-1; k++) printf("%.2f, ", B[j][k]);
                printf("%.2f\n", B[j][k]);
            }
            int res = simplify(B, n, i);
            for (int j = 0; j < n; j++) sendto(sock, B[j], 2*n*sizeof(double), 0, (struct sockaddr *) &name, size);
            sendto(sock, &res, sizeof(int), 0, (struct sockaddr *) &name, size);
            sem_post(write_sem);
            return 0;
        }
    }
    int res = 0, i = 0;
    while(i++ < n && !res) {
        if (sendto(sock, &n, sizeof(int), 0, (struct sockaddr *) &name, size)) {
            perror("sending error");
        }
        for (int j = 0; j < n; j++) sendto(sock, B[j], 2*n*sizeof(double), 0, (struct sockaddr *) &name, size);
        sem_post(read_sem);
        sem_wait(write_sem);
        for (int j = 0; j < n; j++) recvfrom(sock, B[j], 2*n*sizeof(double), 0, NULL, NULL);
        recvfrom(sock, &res, sizeof(int), 0, NULL, NULL);
    }
    if (!res) {
        printf("Обратная матрица\n");
        show_from(B, n, 2*n, 0, n);    
    }
    for(int i=0; i<n; i++) wait(NULL);
    
    sem_unlink(SEM_READ);
    sem_unlink(SEM_WRITE);
}
