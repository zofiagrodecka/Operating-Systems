#include <stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include <unistd.h>
#include<time.h>
#include "function.h"

int main(int argc, char *argv[]) {

    char filename1[N];
    char filename2[N];
    char str1[N];
    char str2[N];

    if(argc <= 1){
        perror("Lack of invocation arguments!");
        exit(-1);
    }
    strcpy(filename1, argv[1]);
    strcpy(filename2, argv[2]);


#ifdef LIB
    clock_t start = clock();

    replace_lib(filename1, filename2, str1, str2);

    clock_t end = clock();
    double time = (end-start)*(1.0/CLOCKS_PER_SEC);

    FILE *results;
    results = fopen("pomiar_zad_4.txt", "a");
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

    replace_sys(filename1, filename2, str1, str2);

    clock_t end = clock();
    double time = (end-start)*(1.0/CLOCKS_PER_SEC);

    FILE *results;
    results = fopen("pomiar_zad_4.txt", "a");
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
