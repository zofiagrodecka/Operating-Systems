#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#define MAX_PROCESSES 9

#define PIPE_PATH_IND 1
#define ROW_IND 2
#define FILE_PATH_IND 3
#define N_IND 4

int main(int argc, char *argv[]) {
    if(argc != 5){
        perror("Wrong number of arguments");
        exit(2);
    }
    printf("Hello, producer!\n");

    int pipe;
    pipe = open(argv[PIPE_PATH_IND], O_WRONLY);

    int file;
    file = open(argv[FILE_PATH_IND], O_RDONLY);

    int N = (int) strtol(argv[N_IND], NULL, 10);

    char tmp[N+1];
    char *str = calloc(N+3, sizeof(char));
    strcpy(str, argv[ROW_IND]);
    int c;
    while((c=read(file, tmp, N)) != 0){
        tmp[c] = 0;
        sleep(1);
        strcat(str, ":");
        strcat(str, tmp);
        printf("PRODUCER PID: %d str: %s\n", getpid(), str);
        write(pipe, str, N+2);
        strcpy(str, argv[ROW_IND]);
    }

    close(file);
    close(pipe);

    free(str);

    return 0;
}
