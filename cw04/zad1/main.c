#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#define SIGNAL SIGUSR1
#define EXEC

int sigcount = 0;

void handler(int signum){
    printf("PID: %d, Otrzymalem sygnal: %d\n", getpid(), signum);
    sigcount++;
}

void blockSignal(){
    sigset_t blockmask;/* sygnały do blokowania */
    sigset_t oldmask; /* aktualna maska sygnałów */
    sigemptyset(&blockmask); /* wyczyść zbiór blokowanych sygnałów */

    sigaddset(&blockmask, SIGNAL); /* dodaj sygnal do zbioru */

    /* Blokowanie*/
    if (sigprocmask(SIG_BLOCK, &blockmask, &oldmask) < 0){
        perror("Cannot block signal");
        exit(3);
    }
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
        printf("PID: %d, Signal: %d is pending\n", getpid(), SIGNAL);
    } else if (inSet == 0) {
        printf("PID: %d, Signal: %d is NOT pending\n", getpid(), SIGNAL);
    } else {
        perror("Error while reading pending set");
        exit(2);
    }
}


int main(int argc, char *argv[]){
   printf("PID: %d, Hello, World!\n", getpid());

    if(argc < 2){
        perror("No invocation arguments");
        exit(1);
    }

    if(strcmp(argv[1], "ignore") == 0){
        signal(SIGNAL, SIG_IGN);
    }
    else if(strcmp(argv[1], "handler") == 0){
        signal(SIGNAL, handler);
    }
    else if(strcmp(argv[1], "mask") == 0 || strcmp(argv[1], "pending") == 0){

        blockSignal();
        checkBlocked();
    }
    else{
        perror("Wrong invocation arguments");
        exit(1);
    }

    raise(SIGNAL);

    if(strcmp(argv[1], "mask") == 0 || strcmp(argv[1], "pending") == 0) {
        checkBlocked();
    }

    int childPID = fork();

    if(childPID == 0){  // jestem w procesie potomnym
#ifndef EXEC
        if(strcmp(argv[1], "mask") == 0 || strcmp(argv[1], "pending") == 0) {
            if(strcmp(argv[1], "mask") == 0){
                raise(SIGNAL);
            }

            checkBlocked();
        }
        else{
            raise(SIGNAL);
        }
#else
        //raise(SIGNAL);
        int res = execvp("./execmain", argv);
        if(res == -1){
            perror("Error with execvp");
            exit(10);
        }
#endif
    }


    return 0;
}
