#include <stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include <unistd.h>
#include "function.h"

void replace_lib(char *inputfile, char *outputfile, char *n1, char *n2){
    FILE * in;
    in = fopen(inputfile, "r");
    if(in == NULL){
        perror("Cannot open file.");
        exit(-1);
    }

    FILE * out;
    out = fopen(outputfile, "w");
    if(out == NULL){
        perror("Cannot open file.");
        exit(-1);
    }

    char str[MAX_LINE_LENGTH] = "";
    char c[2] = "";
    while(fread(c, sizeof(char), 1, in) == 1){
        if(strcmp(c, " ") == 0 || strcmp(c, "\n")==0){
            if(strcmp(str, n1)==0){
                strcpy(str, n2);
            }
            strcat(str, c); // dodanie bialego znaku na koncu
            fwrite(str, sizeof(char), strlen(str), out);
            strcpy(str, "");
        }
        else{
            strcat(str, c);
        }
    }

    fclose(in);
    fclose(out);
}

void replace_sys(char *inputfile, char *outputfile, char *n1, char *n2){
    int in = open(inputfile, O_RDONLY);
    if(in == -1){
        perror("Cannot open file.");
        exit(-1);
    }

    int out;
    out = open(outputfile, O_WRONLY | O_CREAT);
    if(out == -1){
        perror("Cannot open file.");
        exit(-1);
    }

    char str[MAX_LINE_LENGTH] = "";
    char c[2] = "";
    while(read(in, c, sizeof(char)) == 1){
        if(strcmp(c, " ") == 0 || strcmp(c, "\n")==0){
            if(strcmp(str, n1)==0){
                strcpy(str, n2);
            }
            strcat(str, c); // dodanie bialego znaku na koncu
            write(out, str, sizeof(char)*strlen(str));
            strcpy(str, "");
        }
        else{
            strcat(str, c);
        }
    }

    close(in);
    close(out);
}


