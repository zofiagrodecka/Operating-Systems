#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <signal.h>
#include "utils.h"

sem_t *sem_tableIN;
sem_t *sem_tableOUT;
sem_t *sem_table_update;
struct table *table_mem;

void sigint_handler(int signum){
    if(signum == SIGINT){
        if((sem_close(sem_tableIN)) == -1){
            perror("Cannot close tableIN semaphore");
        }

        if((sem_close(sem_tableOUT)) == -1){
            perror("Cannot close tableOUT semaphore");
        }

        if((sem_close(sem_table_update)) == -1){
            perror("Cannot close table_update semaphore");
        }

        if(munmap(table_mem, sizeof(struct table)) == -1){
            perror("Cannot detach table memory");
        }
    }
    exit(130);
}

int main(int argc, char *argv[]) {
    if(argc != 5){
        perror("Wrong number of arguments");
        exit(10);
    }

    printf("PID: %d, Hello, delivery man!\n", getpid());

    signal(SIGINT, sigint_handler);

    if((sem_tableIN = sem_open(argv[1], O_RDWR)) == SEM_FAILED){
        perror("Cannot open tableIN semaphore");
        exit(1);
    }

    if((sem_tableOUT = sem_open(argv[2], O_RDWR)) == SEM_FAILED){
        perror("Cannot open tableOUT semaphore");
        exit(1);
    }

    if((sem_table_update = sem_open(argv[3], O_RDWR)) == SEM_FAILED){
        perror("Cannot open table_update semaphore");
        exit(1);
    }

    int table_desc;
    if((table_desc = shm_open(argv[4], O_RDWR, PERMS)) == -1){
        perror("Cannot openn shared memory for table");
        exit(2);
    }

    // dolaczenie pamieci do przestrzeni adresowej
    if((table_mem = mmap(NULL, sizeof(struct table), PROT_READ | PROT_WRITE, MAP_SHARED, table_desc, 0)) == (struct table *) -1) {
        perror("Cannot attach shared memory");
    }

    int my_pizza_type;
    struct timeval tv;

    while(1){

        decrease_semaphore(sem_tableOUT); // chce zaczac prace ze stolem (czy stol nie jest pusty?)
        decrease_semaphore(sem_table_update);

        my_pizza_type = get_pizza_from_table(table_mem);
        gettimeofday(&tv, NULL);
        printf("(PID: %d, timestamp: %lds:%ldus) Pobieram pizze: %d. Liczba pizz na stole: %d\n", getpid(),
               tv.tv_sec, tv.tv_usec, my_pizza_type, table_mem->n);
        increase_semaphore(sem_table_update);

        increase_semaphore(sem_tableIN); // wiecej wolnego miejsca na stole

        sleep(DELIVERY_TIME);

        gettimeofday(&tv, NULL);
        printf("(PID: %d, timestamp: %lds:%ldus) Dostarczam pizze: %d\n", getpid(), tv.tv_sec, tv.tv_usec, my_pizza_type);

        sleep(DELIVERY_TIME); // powrot
    }

    return 0;
}
