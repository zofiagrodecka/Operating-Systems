#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#define SIGNAL SIGUSR1


void handler(int signum){
    printf("PID: %d, Otrzymalem sygnal: %d\n", getpid(), signum);
}

void checkBlocked(){
    sigset_t pending_set;
    sigemptyset(&pending_set);
    int p = sigpending(&pending_set);

    if (p == -1) {
        perror("Error while reading pending signals");
        exit(2);
    }

    int inSet = sigismember(&pending_set, SIGNAL);
    if (inSet == 1) {
        printf("EXEC MAIN: PID: %d, Signal: %d is pending\n", getpid(), SIGNAL);
    } else if (inSet == 0) {
        printf("EXEC MAIN: PID: %d, Signal: %d is NOT pending\n", getpid(), SIGNAL);
    } else {
        perror("Error while reading pending set");
        exit(2);
    }
}

int main(int argc, char *argv[]){
    printf("PID: %d, Hello in exec\n", getpid());

    if(strcmp(argv[1], "mask") == 0 || strcmp(argv[1], "pending") == 0) {
        if(strcmp(argv[1], "mask") == 0){
            raise(SIGNAL);
        }

        checkBlocked();
    }
    else{
        raise(SIGNAL);
    }

    printf("PID: %d , exiting exec...\n", getpid());

    return 0;
}

