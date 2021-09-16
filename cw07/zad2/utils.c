#include <stdio.h>
#include <sys/sem.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "utils.h"

void decrease_semaphore(sem_t *semaphore){
    if((sem_wait(semaphore)) == -1){
        perror("Cannot decrease semaphore");
    }
}

void increase_semaphore(sem_t *semaphore){
    if(sem_post(semaphore) == -1){
        perror("Cannot increase semaphore");
    }
}

int add_pizza_to_oven(int pizza_type, struct oven *pmem){
    int index;
    int found = 0;

    int i = 0;
    while(!found && i < MAX_OVEN_CAPACITY){
        if(pmem->content[i] == 10){
            found = 1;
            index = i;
            pmem->content[i] = pizza_type;
        }
        i++;
    }

    if(!found){
        perror("No empty slot in oven");
        exit(100);
    }

    pmem->n++;
    return index;
}

void add_pizza_to_table(int pizza_type, struct table *pmem){
    int found = 0;

    int i = 0;
    while(!found && i < MAX_TABLE_CAPACITY){
        if(pmem->content[i] == 10){
            found = 1;
            pmem->content[i] = pizza_type;
        }
        i++;
    }

    if(!found){
        perror("No empty slot in table");
        exit(100);
    }

    pmem->n++;
}

void get_pizza_from_oven(int pizza_index, struct oven *pmem){
    pmem->n--;
    pmem->content[pizza_index] = 10;
}

int get_pizza_from_table(struct table *pmem){
    int found = 0;
    int i = 0;
    int pizza_type;
    while(!found && i < MAX_OVEN_CAPACITY){
        if(pmem->content[i] != 10){
            found = 1;
            pizza_type = pmem->content[i];
            pmem->content[i] = 10;
        }
        i++;
    }

    if(!found){
        perror("No empty slot on the table");
        exit(100);
    }

    pmem->n--;
    return pizza_type;
}
