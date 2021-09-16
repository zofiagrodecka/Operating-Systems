#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include<fcntl.h>
#include <unistd.h>
#include<time.h>
#define N 1000 // max liczba znakow w nazwie pliku

# define SYS

int main(int argc, char *argv[]){

    char filename1[N];
    char filename2[N];

    if(argc <= 1){
        printf("Prosze podac nazwe 1 pliku:\n");
        scanf("%s", filename1);
        printf("Prosze podac nazwe 2 pliku:\n");
        scanf("%s", filename2);
    }
    else{
        strcpy(filename1, argv[1]);
        strcpy(filename2, argv[2]);
    }
    //printf("%s, %s\n", filename1, filename2);

#ifdef LIB
    clock_t start = clock();
    FILE * f1;
    f1 = fopen(filename1, "r");
    if(f1 == NULL){
        perror("Cannot open file.");
        exit(-1);
    }

    FILE * f2;
    f2 = fopen(filename2, "r");
    if(f2 == NULL){
        perror("Cannot open file.");
        exit(-1);
    }

    char c;
    int i = 0;
    FILE *read_file = f1;

    while(fread(&c, sizeof(char), 1, read_file) == 1){
        printf("%c", c);
        if(c == '\n'){
            if(i % 2 == 0){ // wczytalam 1 linijke 1 pliku
                read_file = f2;
            }
            else{
                read_file = f1;
            }
            i++;
        }
    }

    if(read_file == f1){
        while(fread(&c, sizeof(char), 1, f2) == 1){
            printf("%c", c);
        }
    }
    else{
        while(fread(&c, sizeof(char), 1, f1) == 1){
            printf("%c", c);
        }
    }

    fclose(f1);
    fclose(f2);

    clock_t end = clock();
    double time = (end-start)*(1.0/CLOCKS_PER_SEC);

    FILE *results;
    results = fopen("pomiar_zad_1.txt", "a");
    if(results == NULL){
        perror("Cannot open file.");
        exit(-1);
    }

    fprintf(results, "Czas wykonania programu uzywajac funkcji bibliotecznych: %15.50f s\n", time);
    printf("Czas wykonania programu uzywajac funkcji bibliotecznych: %15.50f s\n", time);

    fclose(results);
#endif

#ifdef SYS
    clock_t start = clock();

    int f1;
    f1 = open(filename1, O_RDONLY);
    if(f1 == -1){
        perror("Cannot open file");
        exit(-1);
    }

    int f2;
    f2 = open(filename2, O_RDONLY);
    if(f2 == -1){
        perror("Cannot open file");
        exit(-1);
    }

    char c;
    int i = 0;
    int read_file = f1;

    while(read(read_file, &c, sizeof(char)) == 1){
        printf("%c", c);
        if(c == '\n'){
            if(i % 2 == 0){ // wczytalam 1 linijke 1 pliku
                read_file = f2;
            }
            else{
                read_file = f1;
            }
            i++;
        }
    }

    if(read_file == f1){
        while(read(f2, &c, sizeof(char)) == 1){
            printf("%c", c);
        }
    }
    else{
        while(read(f1, &c, sizeof(char)) == 1){
            printf("%c", c);
        }
    }

    close(f1);
    close(f2);

    clock_t end = clock();
    double time = (end-start)*(1.0/CLOCKS_PER_SEC);

    FILE *results;
    results = fopen("pomiar_zad_1.txt", "a");
    if(results == NULL){
        perror("Cannot open file.");
        exit(-1);
    }

    fprintf(results, "Czas wykonania programu uzywajac funkcji systemowych: %15.50f s\n", time);
    printf("Czas wykonania programu uzywajac funkcji systemowych: %15.50f s\n", time);

    fclose(results);
#endif

    return 0;
}
