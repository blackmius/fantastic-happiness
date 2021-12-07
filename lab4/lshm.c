#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <string.h>
#include <fcntl.h>

// Вывод квадратной матрицы
void show(int **matrix, int n) {
    for (int row = 0; row < n; row++) {
        for(int col = 0; col < n; col++)
            printf("%d\t", matrix[row][col]);
        printf("\n");
    }
}

// Получение матрицы без i-й строки и j-го столбца
void minor(int **mas, int **p, int i, int j, int m) {
    int di = 0;
    for (int ki = 0; ki < m - 1; ki++) { // проверка индекса строки
        if (ki == i) di = 1;
        int dj = 0;
        for (int kj = 0; kj < m - 1; kj++) { // проверка индекса столбца
            if (kj == j) dj = 1;
            p[ki][kj] = mas[ki + di][kj + dj];
        }
    }
}

// Рекурсивное вычисление определителя
int det(int **matrix, int m) {
    int j = 0;
    int d = 0;
    int k = 1; //(-1) в степени i
    if (m < 1) printf("Определитель вычислить невозможно! \n");
    if (m == 1) {
        d = matrix[0][0];
        return d;
    }
    if (m == 2) {
        d = matrix[0][0] * matrix[1][1] - (matrix[1][0] * matrix[0][1]);
        return d;
    }
    if (m > 2) {
        int **mmatrix = malloc(m * sizeof(int*));
        for (int i = 0; i < m; i++) {
            mmatrix[i] = malloc(m * sizeof(int));
        }
        for (int i = 0; i < m; i++) {
            minor(matrix, mmatrix, i, 0, m);
            d = d + k * matrix[i][0] * det(mmatrix, m - 1);
            k = -k;
        }
    }
    return d;
}

void worker(int p[4]) {
    void *ptr[4];
    ptr[0] = mmap(0, sizeof(int), PROT_WRITE, MAP_SHARED, p[0], 0);
    ptr[1] = mmap(0, sizeof(int), PROT_WRITE, MAP_SHARED, p[1], 0);
    
    int n = *(int*) ptr[0];
    int m = *(int*) ptr[1];
    
    ptr[2] = mmap(0, m*m*sizeof(int), PROT_WRITE, MAP_SHARED, p[2], 0);
    ptr[3] = mmap(0, sizeof(int), PROT_WRITE, MAP_SHARED, p[3], 0);
    
    int **mmatrix = malloc(m * sizeof(int*));
    int **matrix = malloc(m * sizeof(int*));
    for (int i = 0; i < m; i++) {
        matrix[i] = malloc(m * sizeof(int));
        mmatrix[i] = malloc(m * sizeof(int));
    }
    
    for (int i = 0; i < m; i++) memcpy(matrix[i], ptr[2] + i*m*sizeof(int), m*sizeof(int));
   
    minor(matrix, mmatrix, n, 0, m);
    int d = matrix[n][0] * det(mmatrix, m - 1);
    *(int*)ptr[3] = d;
}

int main(void) {
    srand(time(NULL));
    int n = 3 + rand() % 5;
    
    int** A = (int**) malloc(n * sizeof(int*));
    for (int row = 0; row < n; row++) {
        A[row] = (int*) malloc(n * sizeof(int));
        for (int col = 0; col < n; col++)
            A[row][col] = rand() % 100;
    }
    printf("Сгенерированная матрица\n");
    show(A, n);
    
    int k = 1;
    int d = 0;
    if (n > 2) {
        int fd_shm[n][4];
        for (int i = 0; i < n; i++) {
            int sizes[4] = {sizeof(int), sizeof(int), n*n*sizeof(int), sizeof(int)};
            void *ptr[4];
            
            for (int j = 0; j < 4; j++) {
                char sh_name[5] = {'s', 'h', 'm', 0x00, 0x00};
                sprintf(&sh_name[3], "%d", 4*i+j);
                if ((fd_shm[i][j] = shm_open(sh_name, O_RDWR | O_CREAT, 0777)) < 0) {
                    perror("error create shm"); exit(1);
                }
                ftruncate(fd_shm[i][j], sizes[j]);
                ptr[j] = mmap(0, sizes[j], PROT_WRITE, MAP_SHARED, fd_shm[i][j], 0);
            }
            
            *(int*)ptr[0] = i;
            *(int*)ptr[1] = n;
            for (int j = 0; j < n; j++) memcpy(ptr[2] + j*n*sizeof(int), A[j], n*sizeof(int));
            
            if (fork() == 0) {
                worker(fd_shm[i]);
                exit(0);
            }
        }
        while (wait(NULL) > 0);
        for (int i = 0; i < n; i++) {
            void *ptr = mmap(0, sizeof(int), PROT_WRITE, MAP_SHARED, fd_shm[i][3], 0);
            d = d + k * *(int*) ptr;
            k = -k;
        }
    } else {
        d = det(A, n);
    }
    
    printf("det A = %d\n", d);
}
