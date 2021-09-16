#include <stdio.h>
#include<stdlib.h>
#include<signal.h>
#include <unistd.h>

#define NODEFER
#define SIGINFO

/*
 * Jesli zdefiniuje tylko SIGINFO to sprawdzana jest flaga SA_SIGINFO i wysylany 1 sygnal SIGUSR1
 * by sprawdzic czy handler go obsluzy
 *
 * Jesli zdefiniuje tylko NOCLDSTOP to sprawdzana jest flaga NOCLDSTOP i wysylam do dziecka z konsoli sygnal
 * SIGSTOP lub SIGCONT, etc i sprawdzam czy handler dziala - powinien sie nie handlowac dla SIGSTOP, etc
 * Jesli zdefiniuje tylko SIGINFO i chce sprawdzic flage NOCLDSTOP to wysylam do dziecka te sygnaly co wyzej
 * i wtedy powinny sie handlowac wszystkie
 * Zeby zakonczyc dzialanie nalezy wyslac SIGTERM z konsoli do rodzica i dziecka
 *
 * Jesli zdefiniuje tylko NODEFER to sprawdzana jest flaga NODEFER i wysylany jest 1 sygnal SIGUSR2
 * i ponowne wyslanie SIGUSR2 z handlera nie bedzie powodowalo zablokowania tego sygnalu
 * Jesli zdefiniuje NODEFER i SIGINFO to wysylany SIGUSR2 i mozna zobaczyc ze jest on blokowany po ponownym go wyslaniu
 * w hanlderze
 */

void checkBlocked(int signal){
    sigset_t pending_set;
    sigemptyset(&pending_set);
    int p = sigpending(&pending_set);

    if (p == -1) {
        perror("Error while reading pending signals");
        exit(2);
    }

    int inSet = sigismember(&pending_set, signal);
    if (inSet == 1) {
        printf("PID: %d, Signal: %d is pending\n", getpid(), signal);
    } else if (inSet == 0) {
        printf("PID: %d, Signal: %d is NOT pending\n", getpid(), signal);
    } else {
        perror("Error while reading pending set");
        exit(2);
    }
}

int counter = 0;

void siginfo_handler(int signum, siginfo_t * info, void * ucontext){
    if(signum == SIGUSR1){
        printf("SIGUSR1: PID: %d, Got signal: %d\n", info->si_pid, signum);
    }
    else if(signum == SIGCHLD){
        /* Jesli do dziecka wysle sygnal SIGSTOP, SIGTSTP, SIGTTIN, SIGTTOU lub SIGCONT
         * i flaga jest ustawiona na SA_NOCLDSTOP
         *  to nie odbiera powiadomien o zatrzymaniu dziecka
         *  Dopiero tylko SIGTERM dziala
         */
        printf("SIGCHLD: PID: %d, Got signal: %d\n", info->si_pid, signum);
    }
    else if(signum == SIGUSR2){
        printf("SIGUSR2: PID: %d, Got signal: %d\n", info->si_pid, signum);
        checkBlocked(SIGUSR2);
        if(counter == 0){
            printf("Raising signal\n");
            counter++;
            raise(SIGUSR2);
            printf("After second raise:\n");
            checkBlocked(SIGUSR2);
        }

        if(counter == 2){
            counter = 0;
        }
    }
}


int main() {
    printf("PID: %d, Hello, World!\n", getpid());
    struct sigaction action;
    action.sa_sigaction = siginfo_handler;
    sigemptyset(&action.sa_mask);

#ifdef SIGINFO
    action.sa_flags = SA_SIGINFO ;
#endif
#ifdef NOCLDSTOP
    action.sa_flags = SA_SIGINFO | SA_NOCLDSTOP;
#endif
#if !defined(SIGINFO) && defined(NODEFER)
    action.sa_flags = SA_SIGINFO | SA_NODEFER;
#endif

    sigaction(SIGUSR1, &action, NULL);
    sigaction(SIGCHLD, &action, NULL);
    sigaction(SIGUSR2, &action, NULL);

#ifdef NOCLDSTOP
    int childPID = fork();

    if(childPID == 0) {  // jestem w procesie potomnym
        printf("CHILDPID: %d\n", getpid());
        while(1) {
            sleep(1);
        }
    }
#endif

#if defined(SIGINFO) && !defined(NODEFER)
    raise(SIGUSR1);
#endif

#if defined(SIGINFO) && defined(NODEFER)
    raise(SIGUSR2);
#endif
#ifdef NODEFER
    raise(SIGUSR2);
#endif

#ifdef NOCLDSTOP
    while(1) {
        sleep(1);
    }
#endif


    return 0;
}
