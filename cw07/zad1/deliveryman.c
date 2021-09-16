#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include "utils.h"

struct table *table_mem;

void sigint_handler(int signum){
    if(signum == SIGINT){
        if(shmdt(table_mem) == -1){
            perror("DELIVERY: Cannot detach table memory");
        }
    }
    exit(0);
}

int main(int argc, char *argv[]) {
    if(argc != 5){
        perror("Wrong number of arguments");
        exit(10);
    }

    printf("PID: %d, Hello, delivery man!\n", getpid());

    signal(SIGINT, sigint_handler);

    int sem_tableIN_key = (int)strtol(argv[1], NULL, 10);
    int sem_tableOUT_key = (int)strtol(argv[2], NULL, 10);
    int sem_table_update_key = (int)strtol(argv[3], NULL, 10);
    int table_key = (int)strtol(argv[4], NULL, 10);

    int sem_tableIN_id;
    if((sem_tableIN_id = semget(sem_tableIN_key, 1, 0)) == -1){
        perror("Cannot open table semaphore");
        exit(1);
    }
    int sem_tableOUT_id;
    if((sem_tableOUT_id = semget(sem_tableOUT_key, 1, 0)) == -1){
        perror("Cannot open table semaphore");
        exit(1);
    }
    int sem_table_update_id;
    if((sem_table_update_id = semget(sem_table_update_key, 1, 0)) == -1){
        perror("Cannot open table update semaphore");
        exit(1);
    }

    int tableID;
    if((tableID = shmget(table_key, sizeof(struct table), 0)) == -1){
        perror("Cannot create shared memory for table");
        exit(2);
    }

    table_mem = shmat(tableID, NULL, 0);

    int my_pizza_type;
    struct timeval tv;

    while(1){

        decrease_semaphore(sem_tableOUT_id); // chce zaczac prace ze stolem (czy stol nie jest pusty?)

        decrease_semaphore(sem_table_update_id);
        my_pizza_type = get_pizza_from_table(table_mem);
        gettimeofday(&tv, NULL);
        printf("(PID: %d, timestamp: %lds:%ldus) Pobieram pizze: %d. Liczba pizz na stole: %d\n", getpid(),
               tv.tv_sec, tv.tv_usec, my_pizza_type, table_mem->n);
        increase_semaphore(sem_table_update_id);
        increase_semaphore(sem_tableIN_id); // wiecej wolnego miejsca na stole

        sleep(DELIVERY_TIME);

        gettimeofday(&tv, NULL);
        printf("(PID: %d, timestamp: %lds:%ldus) Dostarczam pizze: %d\n", getpid(), tv.tv_sec, tv.tv_usec, my_pizza_type);

        sleep(DELIVERY_TIME); // powrot
    }

    return 0;
}
