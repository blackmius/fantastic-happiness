#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

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

void worker(int p[2]) {
    int n, m;
    read(p[0], &n, sizeof(int)); // номер столбца
    read(p[0], &m, sizeof(int)); // длина матрицы
    
    int **mmatrix = malloc(m * sizeof(int*));
    int **matrix = malloc(m * sizeof(int*));
    for (int i = 0; i < m; i++) {
        matrix[i] = malloc(m * sizeof(int));
        mmatrix[i] = malloc(m * sizeof(int));
    }
    
    for (int i = 0; i < m; i++) read(p[0], matrix[i], m*sizeof(int));

    minor(matrix, mmatrix, n, 0, m);
    int d = matrix[n][0] * det(mmatrix, m - 1);
    write(p[1], &d, sizeof(int));
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
        int p[n][2];
        for (int i = 0; i < n; i++) {
            if (pipe(p[i]) == -1) {
                perror("Ошибка вызова pipe"); exit(1);
            }
            write(p[i][1], &i, sizeof(int));
            write(p[i][1], &n, sizeof(int));
            for (int j = 0; j < n; j++) write(p[i][1], A[j], n*sizeof(int));
            if (fork() == 0) {
                worker(p[i]);
                exit(0);
            }
        }
        while (wait(NULL) > 0);
        for (int i = 0; i < n; i++) {
            int dd;
            read(p[i][0], &dd, sizeof(int));
            d = d + k * dd;
            k = -k;
        }
    } else {
        d = det(A, n);
    }
    
    printf("det A = %d\n", d);
}
