#include <stdio.h>
#include<stdlib.h>
#include<dirent.h>
#include<string.h>
#include "dirsearching.h"

int isDirectory(struct dirent * file){
    if(file->d_type == DT_DIR && file->d_type != DT_LNK){
        return 1;
    }
    return 0;
}

int search_file(char *filename, char * pattern){
    FILE * file;
    file = fopen(filename, "r");
    if(file == NULL){
        perror("Cannot open file");
        exit(-10);
    }

    char str[MAX_STRING_LENGTH];
    int res = fscanf(file, "%s", str);
    while(res > 0 && strcmp(str, pattern) != 0){
        res = fscanf(file, "%s", str);
    }

    if(strcmp(str, pattern) == 0){
        return 1;
    }
    return 0;
}