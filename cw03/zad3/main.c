#include <stdio.h>
#include<stdlib.h>
#include<dirent.h>
#include<string.h>
#include<unistd.h>
#include "dirsearching.h"

void search_dir(char * directory, char *pattern, int max_depth, int i, pid_t masterPID) {

    char newPath[MAX_FILENAME_LENGTH];
    char filePath[MAX_FILENAME_LENGTH];

    strcpy(filePath, directory);
    strcpy(newPath, directory);

    if(i == max_depth){
        return;
    }

    pid_t childPID = 1;

    if((childPID = fork()) == -1){
        perror("ERROR: Fork failed");
        exit (-1);
    }

    struct dirent * content;
    DIR * dir;
    dir = opendir(directory);
    if(dir == NULL){
        perror("ERROR: Cannot open a directory");
        exit(-10);
    }

    while ((content = readdir(dir))) {

        strcpy(filePath, directory);
        strcpy(newPath, directory);

        if (strcmp(".", content->d_name) != 0 && strcmp("..", content->d_name) != 0) {
            if (childPID == 0 && strstr(content->d_name, ".txt") != NULL) {
                strcat(filePath, "/");
                strcat(filePath, content->d_name);

                if (search_file(filePath, pattern) == 1) {
                    printf("PID: %d, PPID: %d %s, %s\n", getpid(), getppid(), filePath, content->d_name);
                }
            } else if (getpid() == masterPID && isDirectory(content) == 1) {
                strcat(newPath, "/");
                strcat(newPath, content->d_name);

                search_dir(newPath, pattern, max_depth, i + 1, masterPID);
            }
        }
    }
    closedir(dir );
}

int main(int argc, char *argv[]) {
    if( argc <= 1){
        perror("ERROR: No invocation aguments");
        exit(-1);
    }
    else if(argc > 1 && argc < 4){
        perror("ERROR: Not enough invocation arguments");
        exit(-2);
    }
    else if(argc > 4){
        perror("Too many invocation arguments");
        exit(-3);
    }

    char start_dir[MAX_FILENAME_LENGTH];
    strcpy(start_dir, argv[1]);
    printf("Start_dir: %s\n", start_dir);

    int max_search_depth = (int)strtol(argv[3], NULL, 10);

    pid_t masterPID = getpid();
    printf("ParentID: %d\n", masterPID);

    search_dir(start_dir, argv[2], max_search_depth, 0, masterPID);

    return 0;
}
