#include <stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include <unistd.h>
#include<time.h>
#define N 1000 // max liczba znakow w nazwie pliku
#define MAX_LINE_LENGTH 256
#define LIB

int main(int argc, char *argv[]) {

    char character[2];
    char filename[N];

    strcpy(character, argv[1]);
    strcpy(filename, argv[2]);

    //printf("%s, %s\n", character, filename);

#ifdef LIB
    clock_t start = clock();

    FILE * file;
    file = fopen(filename, "r");
    if(file == NULL){
        perror("Cannot open file.");
        exit(-1);
    }

    char one_line[MAX_LINE_LENGTH] = "";

    char c[2]="";

    while(fread(c, sizeof(char), 1, file) == 1){
        strcat(one_line, c);
        if(strcmp(c, "\n") == 0) {
            if (strstr(one_line, character) != NULL) {
                printf("%s", one_line);
            }
            strcpy(one_line, "");
        }
    }

    fclose(file);

    clock_t end = clock();
    double time = (end-start)*(1.0/CLOCKS_PER_SEC);

    FILE *results;
    results = fopen("pomiar_zad_2.txt", "a");
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
    int file;
    file = open(filename, O_RDONLY);
    if(file == -1){
        perror("Cannot open file.");
        exit(-1);
    }

    char one_line[MAX_LINE_LENGTH] = "";

    char c[2];

    while(read(file, c, sizeof(char)) == 1){
        strcat(one_line, c);
        if(strcmp(c, "\n") == 0) {
            if (strstr(one_line, character) != NULL) {
                printf("%s", one_line);
            }
            strcpy(one_line, "");
        }
    }

    close(file);

    clock_t end = clock();
    double time = (end-start)*(1.0/CLOCKS_PER_SEC);

    FILE *results;
    results = fopen("pomiar_zad_2.txt", "a");
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
