#include <stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include <unistd.h>
#include<time.h>
#define N 1000
#define CHARS_LIMIT 50
#define SYS

int main(int argc, char *argv[]) {

    char filename1[N];
    char filename2[N];

    if(argc <= 1){
        perror("No input arguments!");
        exit(-10);
    }

    strcpy(filename1, argv[1]);
    strcpy(filename2, argv[2]);

#ifdef LIB
    clock_t start = clock();

    FILE * in;
    in = fopen(filename1, "r");
    if(in == NULL){
        perror("Cannot open file.");
        exit(-1);
    }

    FILE * out;
    out = fopen(filename2, "w");
    if(out == NULL){
        perror("Cannot open file.");
        exit(-1);
    }

    int counter = 0;
    char c;
    const char endl = '\n';

    while(fread(&c, sizeof(char), 1, in) == 1){
        counter++;

        if(c != endl || counter != 1){ // zeby nie dodawal pustej nowej linii, gdy w pliku jest 50 znakow + \n w linii
            fwrite(&c, sizeof(char), 1, out);
        }

        if(counter == CHARS_LIMIT) {
            fwrite(&endl, sizeof(char), 1, out);
            counter = 0;
        }
        if(c == endl){
            counter = 0;
        }
    }

    fclose(in);
    fclose(out);

    clock_t end = clock();
    double time = (end-start)*(1.0/CLOCKS_PER_SEC);

    FILE *results;
    results = fopen("pomiar_zad_5.txt", "a");
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

    int in;
    in = open(filename1, O_RDONLY);
    if(in == -1){
        perror("Cannot open file.");
        exit(-1);
    }

    int out;
    out = open(filename2, O_WRONLY | O_CREAT);
    if(out == -1){
        perror("Cannot open file.");
        exit(-1);
    }

    int counter = 0;
    char c;
    const char endl = '\n';

    while(read(in, &c, sizeof(char)) == 1){
        counter++;

        if(c != endl || counter != 1){ // zeby nie dodawal pustej nowej linii, gdy w pliku jest 50 znakow + \n w linii
            write(out, &c, sizeof(char));
        }

        if(counter == CHARS_LIMIT) {
            write(out, &endl, sizeof(char));
            counter = 0;
        }
        if(c == endl){
            counter = 0;
        }
    }

    close(in);
    close(out);

    clock_t end = clock();
    double time = (end-start)*(1.0/CLOCKS_PER_SEC);

    FILE *results;
    results = fopen("pomiar_zad_5.txt", "a");
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
