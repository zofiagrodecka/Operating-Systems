#include <stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include <unistd.h>
#include<time.h>
#include "functions.h"

int main() {

    const char data_file[] = "dane.txt";
    const char a_file[] = "a.txt";
    const char b_file[] = "b.txt";
    const char c_file[] = "c.txt";

#ifdef LIB
    clock_t start = clock();

    FILE * file;
    file = fopen(data_file, "r");
    if(file == NULL){
        perror("Cannot open file.");
        exit(-1);
    }

    FILE * a;
    a = fopen(a_file, "w");
    if(a == NULL){
        perror("Cannot open file.");
        exit(-1);
    }

    FILE * b;
    b = fopen(b_file, "w");
    if(b == NULL){
        perror("Cannot open file.");
        exit(-1);
    }

    FILE * c;
    c = fopen(c_file, "w");
    if(c == NULL){
        perror("Cannot open file.");
        exit(-1);
    }

    char number[MAX_LINE_LENGTH] = "";
    int num;

    char character[2]="";

    int counter = 0;

    while(fread(character, sizeof(char), 1, file) == 1){
        strcat(number, character);

        if(strcmp(character, "\n") == 0) {
            num = strToInt(number);

            if(even(num)){
                counter++;
            }

            if(check_tens_digit(num)){
                fwrite(number, sizeof(char), strlen(number), b);
            }

            if(is_square(num) == 1){
                fwrite(number, sizeof(char), strlen(number), c);
            }

            strcpy(number, "");
        }
    }

    char result[MAX_LINE_LENGTH+30] = "Liczb parzystych jest: ";
    sprintf(number, "%d", counter);
    strcat(result, number);
    fwrite(result, sizeof(char), strlen(result), a);

    fclose(file);
    fclose(a);
    fclose(b);
    fclose(c);

    clock_t end = clock();
    double time = (end-start)*(1.0/CLOCKS_PER_SEC);

    FILE *results;
    results = fopen("pomiar_zad_3.txt", "a");
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
    file = open(data_file, O_RDONLY);
    if(file == -1){
        perror("Cannot open file.");
        exit(-1);
    }

    int a;
    a = open(a_file, O_WRONLY | O_CREAT);
    if(a == -1){
        perror("Cannot open file.");
        exit(-1);
    }

    int b;
    b = open(b_file, O_WRONLY | O_CREAT);
    if(b == -1){
        perror("Cannot open file.");
        exit(-1);
    }

    int c;
    c = open(c_file, O_WRONLY | O_CREAT);
    if(c == -1){
        perror("Cannot open file.");
        exit(-1);
    }

    char number[MAX_LINE_LENGTH] = "";
    int num;

    char character[2]="";

    int counter = 0;

    while(read(file, character, sizeof(char)) == 1){
        strcat(number, character);

        if(strcmp(character, "\n") == 0) {
            num = strToInt(number);

            if(even(num)){
                counter++;
            }

            if(check_tens_digit(num)){
                write(b, number, sizeof(char)*strlen(number));
            }

            if(is_square(num) == 1){
                write(c, number, sizeof(char)*strlen(number));
            }

            strcpy(number, "");
        }
    }

    char result[MAX_LINE_LENGTH+30] = "Liczb parzystych jest: ";
    sprintf(number, "%d", counter);
    strcat(result, number);
    write(a, result, sizeof(char)*strlen(result));

    close(file);
    close(a);
    close(b);
    close(c);

    clock_t end = clock();
    double time = (end-start)*(1.0/CLOCKS_PER_SEC);

    FILE *results;
    results = fopen("pomiar_zad_3.txt", "a");
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
