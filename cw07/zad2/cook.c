#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <signal.h>
#include "utils.h"

sem_t *sem_ovenIN;
sem_t *sem_ovenOUT;
sem_t *sem_tableIN;
sem_t *sem_tableOUT;
sem_t *sem_table_update;
struct oven *oven_mem;
struct table *table_mem;

void sigint_handler(int signum){
    if(signum == SIGINT){
        if((sem_close(sem_ovenIN)) == -1){
            perror("Cannot close ovenIN semaphore");
        }

        if((sem_close(sem_ovenOUT)) == -1){
            perror("Cannot close ovenIN semaphore");
        }

        if((sem_close(sem_tableIN)) == -1){
            perror("Cannot close tableIN semaphore");
        }

        if((sem_close(sem_tableOUT)) == -1){
            perror("Cannot close tableOUT semaphore");
        }

        if((sem_close(sem_table_update)) == -1){
            perror("Cannot close table_update semaphore");
        }

        if(munmap(oven_mem, sizeof(struct oven)) == -1){
            perror("Cannot detach oven memory");
        }

        if(munmap(table_mem, sizeof(struct table)) == -1){
            perror("Cannot detach table memory");
        }
    }
    exit(130);
}

int main(int argc, char *argv[]) {
    if(argc != 8){
        perror("Wrong number of arguments");
        exit(10);
    }

    signal(SIGINT, sigint_handler);

    printf("\nPID: %d, Hello, cook!\n", getpid());

    if((sem_ovenIN = sem_open(argv[1], O_RDWR)) == SEM_FAILED){
        perror("Cannot create semaphore");
        exit(1);
    }

    if((sem_ovenOUT = sem_open(argv[2], O_RDWR)) == SEM_FAILED){
        perror("Cannot create semaphore");
        exit(1);
    }

    if((sem_tableIN = sem_open(argv[3], O_RDWR)) == SEM_FAILED){
        perror("Cannot create semaphore");
        exit(1);
    }

    if((sem_tableOUT = sem_open(argv[4], O_RDWR)) == SEM_FAILED){
        perror("Cannot create semaphore");
        exit(1);
    }

    if((sem_table_update = sem_open(argv[5], O_RDWR)) == SEM_FAILED){
        perror("Cannot create semaphore");
        exit(1);
    }

    int oven_desc;
    if((oven_desc = shm_open(argv[6], O_RDWR, PERMS)) == -1){
        perror("Cannot create shared memory for oven");
        exit(2);
    }

    // dolaczenie pamieci do przestrzeni adresowej
    if((oven_mem = mmap(NULL, sizeof(struct oven), PROT_READ | PROT_WRITE, MAP_SHARED, oven_desc, 0)) == (struct oven *) -1) {
        perror("Cannot attach shared memory");
    }

    int table_desc;
    if((table_desc = shm_open(argv[7], O_RDWR, PERMS)) == -1){
        perror("Cannot create shared memory for table");
        exit(2);
    }

    // dolaczenie pamieci do przestrzeni adresowej
    if((table_mem = mmap(NULL, sizeof(struct table), PROT_READ | PROT_WRITE, MAP_SHARED, table_desc, 0)) == (struct table *) -1) {
        perror("Cannot attach shared memory");
    }

    struct timeval tv;
    gettimeofday(&tv, NULL);
    srand(tv.tv_sec*tv.tv_usec);

    int pizza_type;
    int my_pizza_oven_index;

    int semval;

    while(1){
        pizza_type = rand() % (MAX_PIZZA_TYPE+1);
        gettimeofday(&tv, NULL);
        printf("(PID: %d, timestamp: %lds:%ldus) przygotowuje pizze: %d\n", getpid(), tv.tv_sec, tv.tv_usec, pizza_type);

        // wkladanie pizzy do pieca ------------------------------------------------------------------------------
        decrease_semaphore(sem_ovenIN); // chce zaczac prace z piecem (wpusci jak jest miejsce do wlozenia)
        decrease_semaphore(sem_ovenOUT); // wpusci jak nikt w tym czasie nie wyciaga nic z pieca

        my_pizza_oven_index = add_pizza_to_oven(pizza_type, oven_mem);
        gettimeofday(&tv, NULL);
        printf("(PID: %d, timestamp: %lds:%ldus) dodalem pizze: %d. Liczba pizz w piecu: %d\n", getpid(),
               tv.tv_sec, tv.tv_usec, pizza_type, oven_mem->n);

        increase_semaphore(sem_ovenOUT); // po skonczeniu pracy z piecem (otwiera dla kucharzy wyciagajacych)

        sleep(BAKING_TIME);

        // wyciaganie pizzy z pieca ------------------------------------------------------------------------------
        if((sem_getvalue(sem_ovenOUT, &semval)) == -1){
            perror("Cannot get value of semaphore");
        }
        decrease_semaphore(sem_ovenOUT); // chce zaczac prace z piecem (wkladanie)

        get_pizza_from_oven(my_pizza_oven_index, oven_mem);

        increase_semaphore(sem_ovenIN); // wyjal pizze, wiec jest wiecej miejsca na wkladanie
        increase_semaphore(sem_ovenOUT); // po skonczeniu pracy z piecem (wyszedlem, wiec mozna wkladac do pieca)

        decrease_semaphore(sem_tableIN); // chce zaczac prace ze stolem (mniej wolnego miejsca na stole)

        decrease_semaphore(sem_table_update);

        add_pizza_to_table(pizza_type, table_mem);
        gettimeofday(&tv, NULL);
        printf("(PID: %d, timestamp: %lds:%ldus) Wyjmuje pizze: %d. Liczba pizz w piecu: %d. Liczba pizz na stole: %d\n",
               getpid(), tv.tv_sec, tv.tv_usec, pizza_type, oven_mem->n, table_mem->n);
        increase_semaphore(sem_table_update);

        increase_semaphore(sem_tableOUT); // podnosze by dostawcy mogli brac pizze
    }


    return 0;
}
