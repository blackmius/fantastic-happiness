#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

void show_help() {
    puts("Usage: ./map <FILE> N=0?\n");
    exit(1);
}

int main(int argc, char *argv[]) {
    if (argc > 3 || argc < 2) show_help();
    
    char* fname = argv[1];
    int n = 0;
    
    // если есть аргумент, пытаемся его разобрать
    if (argc == 3) {
        n = atoi(argv[2]);
    }
    
    // Проверяем наличие указанного файла, если его нет
    if (access(fname, F_OK) != 0) {
        puts("Файл с указанным именем не найден.\nУказать другое имя файла? [Y/n]: ");
        
        int c;
        system ("/bin/stty raw"); // включаю считывание любых нажатий
        while((c=getchar()) == 0) putchar(c);
        system ("/bin/stty cooked"); // выключаю считывание любых нажатий
        
        if (c == 'n') exit(0);
        
        puts("\nВведите другое имя файла\n");
        
        fflush(stdin); // чтобы введенная ранее буква не попала в имя файла
        scanf("%s", fname);
        
        // проверять наличие файла, пока пользователь не угадает
        while (access(fname, F_OK) != 0) {
            puts("Файл с указанным именем не найден.\n");
            puts("Введите другое имя файла\n");
            scanf("%s", fname);
        }  
    }
    
    FILE* f = fopen(fname, "r");
    
    // если пользователь указал 0 или не указывал
    if (n != 0) {
        char c;
        while (1) {
            // последняя итерация, которая не выводит символ изза i < n съедает символ,
            // поэтому его надо вывести здесь
            if (c != '\n') putchar(c);
            int i = 0;
            while ((c = fgetc(f)) != -1 && i < n) {
                putchar(c);
                if (c == '\n') i++;
            }
            if (c == -1) break;
            getchar();
       }
    } else {
        char c;
        while ((c = fgetc(f)) != -1) putchar(c);
    }
    
    return 0;
}
