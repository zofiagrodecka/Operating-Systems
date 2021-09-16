#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#define MAX_PID_LENGTH 20
#define MAX_MODE_LENGTH 10

int signals_received = 0;
sigset_t mask, oldmask;
struct sigaction action;
int received_sigusr2 = 0;
char MODE[MAX_MODE_LENGTH] = " ";
union sigval sigvalue;

void catcher_handler(int signum, siginfo_t * info, void * ucontext){
    printf("MY PID: %d, SENDER PID: %d, Otrzymalem sygnal: %d\n", getpid(), info->si_pid, signum);
    if(signum == SIGUSR1 || signum == SIGRTMIN) {
        signals_received++;
        //printf("%d\n", signals_received);
    }
    else if(signum == SIGUSR2 || signum == SIGRTMAX) {
        received_sigusr2 = 1;
        for (int i = 0; i < signals_received; i++) {
            if (strcmp(MODE, "KILL") == 0) {
                kill(info->si_pid, SIGUSR1);
            } else if (strcmp(MODE, "SIGQUEUE") == 0) {
                sigvalue.sival_int = i + 1;
                sigqueue(info->si_pid, SIGUSR1, sigvalue);
            }
            else if (strcmp(MODE, "SIGRT") == 0) {
                kill(info->si_pid, SIGRTMIN);
            }
            //printf("CATCHER sent SIGUSR1\n");
        }

        if (strcmp(MODE, "KILL") == 0) {
            kill(info->si_pid, SIGUSR2);
        } else if (strcmp(MODE, "SIGQUEUE") == 0) {
            sigvalue.sival_int = signals_received;
            sigqueue(info->si_pid, SIGUSR2, sigvalue);
        }
        else if (strcmp(MODE, "SIGRT") == 0) {
            kill(info->si_pid, SIGRTMAX);
        }

        printf("Catcher odebral: %d sygnalow\n", signals_received);
    }
    else{
        perror("Received wrong signal");
        exit(5);
    }
}


int main(int argc, char *argv[]) {

    if(argc < 3){
        perror("Not enough invocation arguments");
        exit(1);
    }

    printf("catcher PID: %d, Hello, World!\n", getpid());

    int myPID = getpid();
    char pid[MAX_PID_LENGTH];
    sprintf(pid, "%d", myPID);
    strcpy(MODE, argv[2]);

    int SIG1 = SIGUSR1;
    int SIG2 = SIGUSR2;

    if(strcmp(MODE,"SIGRT") == 0){
        SIG1 = SIGRTMIN;
        SIG2 = SIGRTMAX;
    }

    int childPID = fork();
    if(childPID < 0){
        perror("Error while fork()");
        exit(3);
    }

    if(childPID == 0){ // w dziecku
        int execres = execl("./sender", "./sender", pid, argv[1], MODE, NULL);
        if(execres == -1){
            perror("Error with execvp");
            exit(10);
        }
    }
    else {
        action.sa_sigaction = catcher_handler;
        action.sa_flags = SA_SIGINFO;

        sigemptyset(&action.sa_mask);
        sigemptyset(&mask);
        sigfillset(&mask); // all signals in mask

        sigdelset(&mask, SIG1);

        action.sa_mask = mask;

        sigdelset(&mask, SIG2);

        sigaction(SIG1, &action, NULL);
        sigaction(SIG2, &action, NULL);

        while (received_sigusr2 == 0) {
            sigsuspend(&mask);
        }
    }

    return 0;
}
