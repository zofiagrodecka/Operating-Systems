#include <stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include "mergelib.h"
#include<sys/times.h>
#include<time.h>
#include<unistd.h>

void create_main_array(struct main_array *mainArray, int size){

    mainArray->pairs_num = size;
    mainArray->index = -1;
    mainArray->pCouple = calloc(size, sizeof(struct files_couple*));
}


struct files_couple *create_couple(char* files){

    struct files_couple *couple = calloc(1, sizeof(struct files_couple));

    char *tmp_file = NULL;
    tmp_file = strtok(files, ":");
    int counter=0;

    while(tmp_file != NULL){ // cos znalazl

        if(counter == 0) {
            couple->filename1 = calloc(strlen(tmp_file)+1, sizeof(char));
            strcpy(couple->filename1, tmp_file);
#ifdef DEBUG
            printf("File1: %s\n", couple->filename1);
#endif
        }
        else if(counter == 1){
            couple->filename2 = calloc(strlen(tmp_file)+1, sizeof(char));
            strcpy(couple->filename2, tmp_file);
#ifdef DEBUG
            printf("File2: %s\n", couple->filename2);
#endif
        }
        else{
            printf("ERROR: Wrong input parameters!");
            exit(-1);
        }

        tmp_file = strtok(NULL, ":");

        counter++;
    }

    return couple;
}

// tworzy plik ze zmergowanymi wierszami na podstawie 2 plikow ze struktury couple
// zwraca ilosc wierszy w pliku wynikowym
int merge_files(char *filename1, char *filename2, char *outputfile){

    //struct tmeasure tstart, tend;

    //get_time(&tstart);

    //sleep(10);

    FILE *file1;
    file1 = fopen(filename1, "r");
    if(file1 == NULL){
        printf("ERROR: Cannot open file: %s\n", filename1);
        exit(-2);
    }

    FILE *file2;
    file2 = fopen(filename2, "r");
    if(file2 == NULL){
        printf("ERROR: Cannot open file: %s\n", filename2);
        exit(-2);
    }

    FILE *outfile;
    outfile = fopen(outputfile, "w");
    if(outfile == NULL){
        printf("ERROR: Cannot open output file: %s\n", outputfile);
        exit(-2);
    }

    char str[MAX_LINE_LENGTH]; // one line
    int i = 0;
    char *fgetsoutput = fgets(str, MAX_LINE_LENGTH, file1);
    while(fgetsoutput != NULL){

        fputs(str, outfile);
        i++;
        if(i % 2 == 0){ // wpisalismy linijke z file1, w nastepnym obiegu wpiszemy do file2
            fgetsoutput = fgets(str, MAX_LINE_LENGTH, file1);
        }else{
            fgetsoutput = fgets(str, MAX_LINE_LENGTH, file2);
        }

    }

    if(i % 2 == 0){
        while(fgets(str, MAX_LINE_LENGTH, file2) != NULL){
            fputs(str, outfile);
            i++;
        }
    }
    else{
        while(fgets(str, MAX_LINE_LENGTH, file1) != NULL){
            fputs(str, outfile);
            i++;
        }
    }

    fclose(file1);
    fclose(file2);
    fclose(outfile);

    //get_time(&tend);
    //report_time("merge_files", tstart, tend);

    return i;
}

// Tworzy blok wierszy
int load_file(char *filename, struct files_couple *couple, int rows_num, struct main_array *mainArray){

    FILE * file;
    file = fopen(filename, "r");
    if(file == NULL){
        printf("ERROR: Cannot open file: %s\n", filename);
        exit(-2);
    }

    couple->block_of_rows = calloc(rows_num, sizeof(char *));

    char *str = NULL;
    str = calloc(MAX_LINE_LENGTH, sizeof(char));
    int i = 0;
    while(fgets(str, MAX_LINE_LENGTH, file) != NULL){
        couple->block_of_rows[i] = str;
        i++;
        str = calloc(MAX_LINE_LENGTH, sizeof(char));
    }

    free(str);
    fclose(file);

    // ustawienie w tablicy glownej wskazania na blok
    mainArray->pCouple[mainArray->index + 1] = couple;
    mainArray->index++;

    return mainArray->index; // indeks elementu tablicy, ktoy zawiera wskazanie na utworzony blok
}

int number_of_rows(int block_index, struct main_array *mainArray){
    return mainArray->pCouple[block_index]->rows_num;
}


void remove_row(int block_index, int row_index, struct main_array *mainArray){

    free(mainArray->pCouple[block_index]->block_of_rows[row_index]);
    mainArray->pCouple[block_index]->block_of_rows[row_index] = NULL;
}


void remove_block(int index, struct main_array *mainArray){

    free(mainArray->pCouple[index]->filename1);
    free(mainArray->pCouple[index]->filename2);

    for(int i=0; i<number_of_rows(index, mainArray); i++){
        //free(mainArray->pCouple[index]->block_of_rows[i]);
        remove_row(index, i, mainArray);
    }

    free(mainArray->pCouple[index]);
    mainArray->pCouple[index] = NULL;

}

void print_couple(struct files_couple *couple){

    for(int i=0; i<couple->rows_num; i++){

        if(couple->block_of_rows[i] != NULL) {
            printf("%s", couple->block_of_rows[i]);
        }
    }
    printf("\n");
}

void print_merged(struct main_array *mainArray){

    for(int i=0; i<mainArray->pairs_num; i++){

        if(mainArray->pCouple[i] != NULL){
            print_couple(mainArray->pCouple[i]);
        }
    }
}

int isNumber(char *string){

    for(int i=0; i<strlen(string); i++){
        if(!isdigit(string[i])){
            return 0;
        }
    }
    return 1;
}


void free_memory(struct main_array *mainArray){

    for(int i=0; i<mainArray->pairs_num; i++){

        if(mainArray->pCouple[i] != NULL) {
            remove_block(i, mainArray);
        }
    }

    free(mainArray->pCouple);
}