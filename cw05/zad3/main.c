#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <unistd.h>

#define N_PROCESSES 5
#define N "30"
#define N_PRODUCENTS
//#define N_CONSUMENTS
//#define BOTH

int main(int argc, char *argv[]) {
    if(argc != 2){
        perror("Wrong number of arguments");
        exit(2);
    }
    printf("PID: %d, Hello, World!\n", getpid());

    mkfifo(argv[1], 0666);

    pid_t parentPID = getpid();
    pid_t childPID = 1;

    pid_t children[N_PROCESSES];
    pid_t mypid;

    for(int i=0; i < N_PROCESSES; i++){
        if(childPID != 0){
            childPID = fork();

            if(getpid() == parentPID){
                children[i] = childPID;
            }
            else {
                children[i] = getpid();
            }
        }
    }

    if(parentPID != (mypid = getpid())){

        printf("My pid: %d\n", mypid);

        for(int myind=0; myind < N_PROCESSES; myind++) {
            if (mypid == children[myind]){
                switch(myind){
                    case 0:
#if defined(N_PRODUCENTS) || defined(BOTH)
                        execl("producer", "producer", argv[1], "1", "file1.txt", N, NULL);
#elif defined(N_CONSUMENTS)
                        execl("consumer", "consumer", argv[1], "res1.txt", N, NULL);
#endif
                        break;
                        case 1:
#if defined(N_PRODUCENTS) || defined(BOTH)
                            execl("producer", "producer", argv[1], "2", "file2.txt", N, NULL);
#elif defined(N_CONSUMENTS)
                            execl("consumer", "consumer", argv[1], "res2.txt", N, NULL);
#endif
                        break;
                    case 2:
#if defined(N_PRODUCENTS) || defined(BOTH)
                        execl("producer", "producer", argv[1], "3", "file3.txt", N, NULL);
#elif defined(N_CONSUMENTS)
                        execl("consumer", "consumer", argv[1], "res3.txt", N, NULL);
#endif
                        break;
                    case 3:
#if defined(N_PRODUCENTS) || defined(BOTH)
                        execl("producer", "producer", argv[1], "4", "file4.txt", N, NULL);
#elif defined(N_CONSUMENTS)
                        execl("consumer", "consumer", argv[1], "res4.txt", N, NULL);
#endif
                        break;
                    case 4:
#if defined(N_PRODUCENTS)
                        execl("producer", "producer", argv[1], "5", "file5.txt", N, NULL);
#elif defined(N_CONSUMENTS)
                        execl("consumer", "consumer", argv[1], "res5.txt", N, NULL);
#elif defined(BOTH)
                        execl("consumer", "consumer", argv[1], "res2.txt", N, NULL);
#endif
                        break;
                }
                return 0;
            }
        }
    }

    //rodzic
#if defined(N_PRODUCENTS) || defined(BOTH)
    execl("consumer", "consumer", argv[1], "res1.txt", N, NULL);
#elif defined(N_CONSUMENTS)
    execl("producer", "producer", argv[1], "1", "file2.txt", N, NULL);
#endif


    return 0;
}
