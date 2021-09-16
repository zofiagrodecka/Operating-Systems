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

struct oven *oven_mem;
struct table *table_mem;

void sigint_handler(int signum){
    if(signum == SIGINT){
        if(shmdt(oven_mem) == -1){
            perror("COOK: Cannot detach oven memory");
        }

        if(shmdt(table_mem) == -1){
            perror("COOK: Cannot detach table memory");
        }
    }
    exit(0);
}


int main(int argc, char *argv[]) {
    printf("PID: %d, Hello, cook!\n", getpid());

    if(argc != 8){
        perror("Wrong number of arguments");
        exit(10);
    }

    signal(SIGINT, sigint_handler);

    int sem_ovenIN_key = (int)strtol(argv[1], NULL, 10);
    int sem_ovenOUT_key = (int)strtol(argv[2], NULL, 10);
    int sem_tableIN_key = (int)strtol(argv[3], NULL, 10);
    int sem_tableOUT_key = (int)strtol(argv[4], NULL, 10);
    int sem_table_update_key = (int)strtol(argv[5], NULL, 10);
    int oven_key = (int)strtol(argv[6], NULL, 10);
    int table_key = (int)strtol(argv[7], NULL, 10);

    int sem_ovenIN_id;
    if((sem_ovenIN_id = semget(sem_ovenIN_key, 1, 0)) == -1){
        perror("Cannot open oven semaphore");
        exit(1);
    }
    int sem_ovenOUT_id;
    if((sem_ovenOUT_id = semget(sem_ovenOUT_key, 1, 0)) == -1){
        perror("Cannot open oven semaphore");
        exit(1);
    }

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

    int ovenID;
    if((ovenID = shmget(oven_key, sizeof(struct oven), 0)) == -1){
        perror("Cannot create shared memory for oven");
        exit(2);
    }

    int tableID;
    if((tableID = shmget(table_key, sizeof(struct table), 0)) == -1){
        perror("Cannot create shared memory for table");
        exit(2);
    }


    // dolaczenie pamieci do przestrzeni adresowej
    oven_mem = shmat(ovenID, NULL, 0);
    table_mem = shmat(tableID, NULL, 0);

    struct timeval tv;
    gettimeofday(&tv, NULL);
    srand(tv.tv_sec*tv.tv_usec);

    int pizza_type;
    int my_pizza_oven_index;

    while(1){
        pizza_type = rand() % (MAX_PIZZA_TYPE+1);
        gettimeofday(&tv, NULL);
        printf("(PID: %d, timestamp: %lds:%ldus) przygotowuje pizze: %d\n", getpid(), tv.tv_sec, tv.tv_usec, pizza_type);

        // wkladanie pizzy do pieca ------------------------------------------------------------------------------
        decrease_semaphore(sem_ovenIN_id); // chce zaczac prace z piecem (wpusci jak jest miejsce do wlozenia)
        decrease_semaphore(sem_ovenOUT_id); // wpusci jak nikt w tym czasie nie wyciaga nic z pieca

        gettimeofday(&tv, NULL);
        my_pizza_oven_index = add_pizza_to_oven(pizza_type, oven_mem);
        printf("(PID: %d, timestamp: %lds:%ldus) dodalem pizze: %d. Liczba pizz w piecu: %d\n", getpid(),
               tv.tv_sec, tv.tv_usec, pizza_type, oven_mem->n);

        increase_semaphore(sem_ovenOUT_id); // po skonczeniu pracy z piecem (otwiera dla kucharzy wyciagajacych)

        sleep(BAKING_TIME);

        // wyciaganie pizzy z pieca ------------------------------------------------------------------------------
        decrease_semaphore(sem_ovenOUT_id); // chce zaczac prace z piecem (wkladanie)

        get_pizza_from_oven(my_pizza_oven_index, oven_mem);

        increase_semaphore(sem_ovenIN_id); // wyjal pizze, wiec jest wiecej miejsca na wkladanie
        increase_semaphore(sem_ovenOUT_id); // po skonczeniu pracy z piecem (wyszedlem, wiec mozna wkladac do pieca)

        decrease_semaphore(sem_tableIN_id); // chce zaczac prace ze stolem (mniej wolnego miejsca na stole)

        decrease_semaphore(sem_table_update_id);
        gettimeofday(&tv, NULL);
        add_pizza_to_table(pizza_type, table_mem);
        printf("(PID: %d, timestamp: %lds:%ldus) Wyjmuje pizze: %d. Liczba pizz w piecu: %d. Liczba pizz na stole: %d\n",
               getpid(), tv.tv_sec, tv.tv_usec, pizza_type, oven_mem->n, table_mem->n);
        increase_semaphore(sem_table_update_id);

        increase_semaphore(sem_tableOUT_id); // podnosze by dostawcy mogli brac pizze

    }

    return 0;
}