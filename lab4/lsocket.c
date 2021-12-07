#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stddef.h>

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

void worker(int sock, struct sockaddr_un addr, size_t size) {    
    int n, m;
    read(sock, &n, sizeof(int));
    read(sock, &m, sizeof(int));
    
    int **mmatrix = malloc(m * sizeof(int*));
    int **matrix = malloc(m * sizeof(int*));
    for (int i = 0; i < m; i++) {
        matrix[i] = malloc(m * sizeof(int));
        mmatrix[i] = malloc(m * sizeof(int));
    }
    
    for (int i = 0; i < m; i++) read(sock, matrix[i], m*sizeof(int));

    minor(matrix, mmatrix, n, 0, m);
    int d = matrix[n][0] * det(mmatrix, m - 1);
    
    if (send(sock, &d, sizeof(int), 0) < 0) {
        perror("sending datagram message");
    }
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
        int socks[n];
        for (int i = 0; i < n; i++) {
            socks[i] = socket(AF_UNIX, SOCK_DGRAM, 0);
            struct sockaddr_un addr;
            memset(&addr, 0, sizeof(addr));
            addr.sun_family = AF_UNIX;
            size_t size = (offsetof (struct sockaddr_un, sun_path) + strlen(addr.sun_path));
            char sn[5] = {'s', 0x00, 0x00};
            sprintf(&sn[1], "%d", i);
            strcpy(addr.sun_path, sn);
		    unlink(sn);
            if (bind(socks[i], (struct sockaddr *) &addr, sizeof(addr)) < 0) {
                perror("binding name to datagram socket");
                exit(1);
            }
            if (connect(socks[i], (struct sockaddr *) &addr, sizeof(addr)) < 0) {
                perror("connect");
                exit(1);
            }
            if (send(socks[i], &i, sizeof(int), 0) < 0) {
                perror("sending datagram message");
            }
            if (send(socks[i], &n, sizeof(int), 0) < 0) {
                perror("sending datagram message");
            }
            for (int j = 0; j < n; j++)    
                if (send(socks[i], A[j], n*sizeof(int), 0) < 0) {
                    perror("sending datagram message");
                }
            if (fork() == 0) {
                worker(socks[i], addr, size);
                exit(0);
            }
        }
        while (wait(NULL) > 0);
        for (int i = 0; i < n; i++) {
            int dd;
            read(socks[i], &dd, sizeof(int));
            d = d + k * dd;
            k = -k;
        }
    } else {
        d = det(A, n);
    }
    
    printf("det A = %d\n", d);
}
