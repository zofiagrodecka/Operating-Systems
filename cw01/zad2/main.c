#include <stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include "../zad1/mergelib.h"
#include<sys/time.h>
#include<sys/times.h>
#include<time.h>
#include<unistd.h>

struct main_array array;

struct tmeasure {
    clock_t realtime;
    clock_t usertime;
    clock_t systime;
};

void get_time(struct tmeasure *t){
    struct tms tms0;

    if ((t->realtime = times (&tms0)) == -1)
        perror ("times");
    t->systime = tms0.tms_stime;
    t->usertime = tms0.tms_utime;
}

void report_time(char *msg, struct tmeasure tstart, struct tmeasure tend){
    FILE *fl = NULL;

    fl = fopen("raport2.txt", "a");

    printf ("%s: measured times real/user/system: %ld/%ld/%ld msec\n",
            msg,
            (tend.realtime-tstart.realtime)*1000/sysconf(_SC_CLK_TCK),
            (tend.usertime-tstart.usertime)*1000/sysconf(_SC_CLK_TCK),
            (tend.systime-tstart.systime)*1000/sysconf(_SC_CLK_TCK) );

    fprintf (fl, "%s: measured times real/user/system: %ld/%ld/%ld msec\n",
             msg,
             (tend.realtime-tstart.realtime)*1000/sysconf(_SC_CLK_TCK),
             (tend.usertime-tstart.usertime)*1000/sysconf(_SC_CLK_TCK),
             (tend.systime-tstart.systime)*1000/sysconf(_SC_CLK_TCK) );

    fclose(fl);
}

int main(int argc, char *argv[]) {

    FILE *raport = NULL;
    raport = fopen("raport2.txt", "w");
    fclose(raport);

    struct tmeasure tstart, tend;

    if(argc <= 1){ // brak argumentow wywolania
        printf("Lack of invocation arguments!");
        exit(-3);
    }

    int i=1;
    enum operation oper = none;
    int n = 0;
    int counter = 0;
    int blockInd = -1;
    char *files = NULL;
    struct files_couple *filesCouple = NULL;

    char *outputfilename = calloc(40, sizeof(char));
    outputfilename[0] = 0;
    char fileindex[20];
    char *txt = ".txt";

    while(i<argc){

        if(strcmp(argv[i], "create_table") == 0){
            oper = createTable;
            counter = 0;
        }
        else if(strcmp(argv[i], "merge_files") == 0){
            oper = mergeFiles;
            counter = 0;
        }
        else if(strcmp(argv[i], "remove_block") == 0){
            oper = removeBlock;
            counter = 0;
        }
        else if(strcmp(argv[i], "remove_row") == 0){
            oper = removeRow;
            counter = 0;
        }
        else{
            switch(oper){

                case createTable:

                    if(counter > 0){
                        printf("ERROR: Too many arguments after create_table.\n");
                        exit(-3);
                    }

                    if(array.pCouple != NULL){ // was created before
                        printf("ERROR: Create_table called the second time.\n");
                        exit(-3);
                    }

                    n = atoi(argv[i]);
                    if(!isNumber(argv[i])){
                        printf("ERROR: Wrong input string: %s\n Should've been size of the array to be created.\n", argv[i]);
                        exit(-3);
                    }

                    printf("Creating table of size: %d\n", n);

                    get_time(&tstart);

                    create_main_array(&array, n);

                    get_time(&tend);
                    report_time("create_table", tstart, tend);

                    counter++;
                    break;

                case mergeFiles:

                    printf("Merging files: \n");

                    if(counter > array.pairs_num-1){
                        printf("ERROR: Too many file couples.\n");
                        exit(-3);
                    }

                    if(array.pCouple == NULL){
                        printf("ERROR: Create_table wasn't called before merge_files.\n");
                        exit(-3);
                    }

                    files = calloc(strlen(argv[i]), sizeof(char));
                    strcpy(files, argv[i]);

                    get_time(&tstart);

                    filesCouple = create_couple(files);
                    sprintf(fileindex, "%d", i);
                    strcpy(outputfilename, "merged");
                    strcat(strcat(outputfilename, fileindex), txt); // outputfilename
                    filesCouple->rows_num = merge_files(filesCouple->filename1, filesCouple->filename2, outputfilename);
                    load_file(outputfilename, filesCouple, filesCouple->rows_num, &array);

                    get_time(&tend);
                    report_time("merge_files", tstart, tend);

                    counter++;
                    break;

                case removeBlock:

                    if(counter > 0){
                        printf("ERROR: Too many arguments after remove_block.\n");
                        exit(-3);
                    }

                    n = atoi(argv[i]);
                    if(!isNumber(argv[i])){
                        printf("ERROR: Wrong input string: %s\n Should've been an index of the block to be removed.\n", argv[i]);
                        exit(-3);
                    }

                    printf("Removing block: %d\n", n);

                    get_time(&tstart);

                    remove_block(n, &array);

                    get_time(&tend);
                    report_time("remove_block", tstart, tend);

                    counter++;
                    break;

                case removeRow:

                    n = atoi(argv[i]);
                    if(!isNumber(argv[i])){
                        printf("ERROR: Wrong input string: %s\n Should've been a number.\n", argv[i]);
                        exit(-3);
                    }

                    if(counter == 0){
                        blockInd = n;
                    }
                    else if(counter == 1){
                        printf("Removing row: %d from %d-th block\n", n, blockInd);

                        get_time(&tstart);

                        remove_row(blockInd, n, &array);

                        get_time(&tend);
                        report_time("remove_row", tstart, tend);
                    }
                    else{
                        printf("ERROR: Too many arguments after remove_row.\n");
                        exit(-3);
                    }
                    counter++;
                    break;

                default:
                    printf("ERROR: Wrong input string: %s\n Should've been operation specification.\n", argv[i]);
                    exit(-3);
            }
        }

        i++;
    }

    free_memory(&array);
    free(outputfilename);

    return 0;
}


