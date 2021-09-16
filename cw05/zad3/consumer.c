#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define PIPE_PATH_IND 1
#define FILE_PATH_IND 2
#define N_IND 3
#define MAX_PROCESSES 9
#define MAX_LINE_LENGTH 10000

int main(int argc, char *argv[]) {
    if(argc != 4){
        perror("Wrong number of arguments");
        exit(2);
    }
    printf("Hello, consumer!\n");

    int pipe;
    pipe = open(argv[PIPE_PATH_IND], O_RDONLY);

    FILE *outfile;
    outfile = fopen(argv[FILE_PATH_IND], "w");

    int N = (int) strtol(argv[N_IND], NULL, 10);

    char *str = calloc(N+5, sizeof(char));
    char *pstr = NULL;
    char *row;
    int r;

    int max = 0;

    char out_tab[MAX_PROCESSES][MAX_LINE_LENGTH+2];
    int c;
    while((c = read(pipe, str, N+2)) != 0){
        str[c] = 0;
        row = strtok(str, ":");
        r = (int) strtol(row, NULL, 10);
        if(r > max){
            max = r;
        }
        pstr = strtok(NULL, ":");
        printf("CONSUMER PID: %d, pstr: %s\n", getpid(), pstr);
        strcat(out_tab[r-1],pstr);
        printf("output_tab: %s\n", out_tab[r-1]);
    }

    printf("Result:\n");
    for(int i=0; i<max; i++){
        puts(out_tab[i]);
        if(out_tab[i][strlen(out_tab[i])-1] != '\n'){
            strcat(out_tab[i], "\n");
        }
        fputs(out_tab[i], outfile);
    }

    fclose(outfile);
    free(str);

    return 0;
}
