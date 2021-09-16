#ifndef header_h
#define header_h

#include <stdio.h>
#include<stdlib.h>

#define MAX_LINE_LENGTH 1000

#define DEBUG

struct files_couple{

    char *filename1;
    char *filename2;
    int rows_num;
    char **block_of_rows;
};


struct main_array{
    int index; // indeks ostatniego dodanego elementu
    int pairs_num;
    struct files_couple **pCouple;
};

enum operation{
    createTable,
    mergeFiles,
    removeBlock,
    removeRow,
    none
};

void create_main_array(struct main_array *mainArray, int size);
struct files_couple *create_couple(char* files);
int merge_files(char *filename1, char *filename2, char *outputfile);
int load_file(char *filename, struct files_couple *couple, int rows_num, struct main_array *mainArray);
int number_of_rows(int block_index, struct main_array *mainArray);
void remove_row(int block_index, int row_index, struct main_array *mainArray);
void remove_block(int index, struct main_array *mainArray);
void print_couple(struct files_couple *couple);
void print_merged(struct main_array *mainArray);
int isNumber(char *string);
void free_memory(struct main_array *mainArray);

#endif // header_h