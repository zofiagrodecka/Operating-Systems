#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

enum mode{
    KILL,
    SIGQUEUE,
    SIGRT
};

sigset_t mask, oldmask;
int signals_received = 0;
int received_sigusr2 = 0;
int should_receive = 0;

void sender_handler(int signum, siginfo_t * info, void * ucontext){
    printf("SENDER: PID: %d, Otrzymalem sygnal: %d\n", getpid(), signum);
    if(signum == SIGUSR1 || signum == SIGRTMIN) {
        signals_received++;
    }
    else if(signum == SIGUSR2 || signum == SIGRTMAX) {
        received_sigusr2 = 1;
        should_receive = info->si_value.sival_int;
    }
    else{
        perror("Received wrong signal");
        exit(5);
    }
}

int main(int argc, char *argv[]) {
    sleep(1);
    if(argc < 4){
        perror("Not enough invocation arguments");
        exit(1);
    }

    int catchedPID = (int)strtol(argv[1], NULL, 10);
    int to_be_sent = (int)strtol(argv[2], NULL, 10);

    int SIG1 = SIGUSR1;
    int SIG2 = SIGUSR2;

    enum mode m;
    if(strcmp(argv[3], "KILL") == 0){
        m = KILL;
    }
    else if(strcmp(argv[3], "SIGQUEUE") == 0){
        m = SIGQUEUE;
    }
    else if(strcmp(argv[3], "SIGRT") == 0){
        m = SIGRT;
        SIG1 = SIGRTMIN;
        SIG2 = SIGRTMAX;
    }

    struct sigaction sender_action;

    sender_action.sa_sigaction = sender_handler;
    sender_action.sa_flags = SA_SIGINFO;

    sigemptyset(&sender_action.sa_mask);
    sigemptyset(&mask);
    sigfillset(&mask); // all signals in mask

    sigdelset(&mask, SIG1);

    sender_action.sa_mask = mask;

    sigdelset(&mask, SIGUSR2);

    sigaction(SIG1, &sender_action, NULL);
    sigaction(SIG2, &sender_action, NULL);

    printf("sender PID: %d, Hello, World!\n", getpid());

    union sigval sigvalue;

    for(int i=0; i<to_be_sent; i++){
        switch(m){
            case KILL:
                kill(catchedPID, SIGUSR1);
                break;
            case SIGQUEUE:
                sigvalue.sival_int = i+1;
                sigqueue(catchedPID, SIGUSR1, sigvalue);
                break;
            case SIGRT:
                kill(catchedPID, SIGRTMIN);
                break;
        }
        //printf("SENDER sent SIGUSR1\n");
    }

    switch(m){
        case KILL:
            kill(catchedPID, SIGUSR2);
            break;
        case SIGQUEUE:
            sigvalue.sival_int = to_be_sent;
            sigqueue(catchedPID, SIGUSR2, sigvalue);
            break;
        case SIGRT:
            kill(catchedPID, SIGRTMAX);
            break;
    }


    while (received_sigusr2 == 0) {
        sigsuspend(&mask);
    }

    sigfillset(&mask); // all signals in mask
    //blokowanie wszystkich sygnalow
    if(sigprocmask(SIG_BLOCK, &mask, &oldmask) < 0){
        perror("Cannot block these signals");
        exit(2);
    }

    printf("Sender odebral: %d sygnalow, wyslal: %d sygnalow\n", signals_received, to_be_sent);
    if(m == SIGQUEUE){
        printf("Sender mial otrzymac: %d sygnalow\n", should_receive);
    }

    return 0;
}

