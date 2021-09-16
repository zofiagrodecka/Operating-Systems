#include <stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<unistd.h>

int main(int argc, char* argv[]) {
    if(argc <= 1){
        perror("No invocation arguments");
        exit(-1);
    }

    int n = strtol(argv[1], NULL, 10); // liczba procesow potomnych
    printf("PID glownego programu: %d\n", (int)getpid());

    pid_t childPID = 1;
    pid_t parentPID = getpid();
    for(int i=0; i<n; i++){

        if(childPID != 0){
            childPID = fork();
        }
    }

    if(getpid() != parentPID){
        printf("My pid: %d, my parent: %d\n", getpid(), getppid());
    }

    return 0;
}
