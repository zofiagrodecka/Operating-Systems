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
int signals_sent= 0;
int received_sigusr2 = 0;
int should_receive = 0;
enum mode m;
union sigval sigvalue;
int catchedPID = 0;

void sender_handler(int signum, siginfo_t * info, void * ucontext){
    printf("SENDER: PID: %d, Otrzymalem sygnal: %d\n", getpid(), signum);
    if(signum == SIGUSR1 || signum == SIGRTMIN) {
        signals_sent++;
        //printf("%d\n", signals_sent);
        switch(m){
            case KILL:
                kill(catchedPID, SIGUSR1);
                break;
            case SIGQUEUE:
                sigvalue.sival_int = signals_sent;
                sigqueue(catchedPID, SIGUSR1, sigvalue);
                break;
            case SIGRT:
                kill(catchedPID, SIGRTMIN);
                break;
        }
        //printf("SENDER sent SIGUSR1\n");

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

    catchedPID = (int)strtol(argv[1], NULL, 10);
    int to_be_sent = (int)strtol(argv[2], NULL, 10);

    int SIG1 = SIGUSR1;
    int SIG2 = SIGUSR2;


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

    switch(m){
        case KILL:
            kill(catchedPID, SIGUSR1);
            break;
        case SIGQUEUE:
            sigvalue.sival_int = signals_sent;
            sigqueue(catchedPID, SIGUSR1, sigvalue);
            break;
        case SIGRT:
            kill(catchedPID, SIGRTMIN);
            break;
    }
    signals_sent++;

    while (signals_sent < to_be_sent) {
        sigsuspend(&mask);
    }

    switch (m) {
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

    printf("Sender wyslal: %d sygnalow\n", signals_sent);

    return 0;
}

