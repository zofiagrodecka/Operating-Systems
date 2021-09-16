#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void sighandler(int signum, siginfo_t *info, void *vcontext){
    printf("Otrzymalem sygnal: %d z wartoscia: %d\n", signum, info->si_value.sival_int);
    /*if(signum == SIGUSR1){

    }
    else if(signum == SIGUSR2){
        printf("Otrzymalem SIGUSR2: %d z wartoscia: %d\n", signum, info->si_value.sival_int);
    }*/
}


int main(int argc, char* argv[]) {

    if(argc != 3){
        printf("Not a suitable number of program parameters\n");
        return 1;
    }

    struct sigaction action;
    action.sa_sigaction = &sighandler;

    //..........
    action.sa_flags = SA_SIGINFO;
    sigaction(SIGUSR1, &action, NULL);
    sigaction(SIGUSR2, &action, NULL);

    sigset_t old_mask;
    sigset_t blocked_signals;
    sigemptyset(&blocked_signals);
    sigfillset(&blocked_signals);
    sigdelset(&blocked_signals, SIGUSR1);
    sigdelset(&blocked_signals, SIGUSR2);
    sigprocmask(SIG_BLOCK, &blocked_signals, &old_mask);

    int child = fork();
    if(child == 0) {
        //zablokuj wszystkie sygnaly za wyjatkiem SIGUSR1 i SIGUSR2
        //zdefiniuj obsluge SIGUSR1 i SIGUSR2 w taki sposob zeby proces potomny wydrukowal
        //na konsole przekazana przez rodzica wraz z sygnalami SIGUSR1 i SIGUSR2 wartosci
        sleep(1);

    }
    else {
        //wyslij do procesu potomnego sygnal przekazany jako argv[2]
        //wraz z wartoscia przekazana jako argv[1]
        int sig = (int)strtol(argv[2], NULL, 10);
        int value = (int)strtol(argv[1], NULL, 10);

        union sigval sigval;
        sigval.sival_int = value;
        sigqueue(child, sig, sigval);

    }

    return 0;
}
