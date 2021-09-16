#include <stdio.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "utils.h"

pid_t children[MAX_N_WORKERS];
int N=0;

char *sem_ovenIN_name = "/ovenIN";
sem_t *sem_ovenIN;
char *sem_ovenOUT_name = "/ovenOUT";
sem_t *sem_ovenOUT;
char *sem_tableIN_name = "/tableIN";
sem_t *sem_tableIN;
char *sem_tableOUT_name = "/tableOUT";
sem_t *sem_tableOUT;
char *sem_table_update_name = "/table_update";
sem_t *sem_table_update;
struct oven *oven_mem;
char *mem_oven_name = "/oven";
int oven_desc;
struct table *table_mem;
char *mem_table_name = "/table";
int table_desc;


void before_exit(){
    if((sem_close(sem_ovenIN)) == -1){
        perror("Cannot close ovenIN semaphore");
    }
    if((sem_unlink(sem_ovenIN_name)) == -1){
        perror("Cannot unlink ovenIN semaphore");
    }

    if((sem_close(sem_ovenOUT)) == -1){
        perror("Cannot close ovenIN semaphore");
    }
    if((sem_unlink(sem_ovenOUT_name)) == -1){
        perror("Cannot unlink ovenOUT semaphore");
    }

    if((sem_close(sem_tableIN)) == -1){
        perror("Cannot close tableIN semaphore");
    }
    if((sem_unlink(sem_tableIN_name)) == -1){
        perror("Cannot unlink tableIN semaphore");
    }

    if((sem_close(sem_tableOUT)) == -1){
        perror("Cannot close tableOUT semaphore");
    }
    if((sem_unlink(sem_tableOUT_name)) == -1){
        perror("Cannot unlink tableOUT semaphore");
    }

    if((sem_close(sem_table_update)) == -1){
        perror("Cannot close table_update semaphore");
    }
    if((sem_unlink(sem_table_update_name)) == -1){
        perror("Cannot unlink table_update semaphore");
    }

    if(munmap(oven_mem, sizeof(struct oven)) == -1){
        perror("Cannot detach oven memory");
    }

    if(munmap(table_mem, sizeof(struct table)) == -1){
        perror("Cannot detach table memory");
    }

    if((shm_unlink(mem_oven_name)) == -1){
        perror("Cannot delete memory for oven");
    }

    if((shm_unlink(mem_table_name)) == -1){
        perror("Cannot delete memory for table");
    }
}

void sigint_handler(int signum){
    if(signum == SIGINT){
        for(int i=0; i<N; i++){
            kill(children[i], SIGINT);
        }

        while(wait(NULL) > 0) {};
        printf("Dzieci sie skonczyly\n");
    }

    before_exit();
    exit(0);
}


int main(int argc, char *argv[]) {
    if(argc != 3){
        perror("Wrong number of arguments");
        exit(10);
    }

    printf("PID: %d, Hello, World!\n", getpid());

    signal(SIGINT, sigint_handler);
    pid_t parentPID = getpid();

    int semval = 0;

    int n_cooks = (int)strtol(argv[1], NULL, 10);
    int n_delivers = (int)strtol(argv[2], NULL, 10);

    // initialization
    // oven semaphores ----------------------------------------------------------------------
    if((sem_ovenIN = sem_open(sem_ovenIN_name, O_RDWR, PERMS, MAX_OVEN_CAPACITY)) != SEM_FAILED){
        perror("Semaphore exists");
        if((sem_close(sem_ovenIN)) == -1){
            perror("Cannot close ovenIN semaphore");
        }
        if((sem_unlink(sem_ovenIN_name)) == -1){
            perror("Cannot unlink ovenIN semaphore");
        }
    }

    if((sem_ovenIN = sem_open(sem_ovenIN_name, O_CREAT | O_RDWR | O_EXCL, PERMS, MAX_OVEN_CAPACITY)) == SEM_FAILED){
        perror("Cannot create semaphore ovenIN");
        exit(1);
    }

    if((sem_getvalue(sem_ovenIN, &semval)) == -1){
        perror("Cannot get value of semaphore");
    }
    printf("semval: %d\n", semval);

    if((sem_ovenOUT = sem_open(sem_ovenOUT_name, O_RDWR, PERMS, 1)) != SEM_FAILED){
        perror("Semaphore exists");
        if((sem_close(sem_ovenOUT)) == -1){
            perror("Cannot close ovenIN semaphore");
        }
        if((sem_unlink(sem_ovenOUT_name)) == -1){
            perror("Cannot unlink ovenOUT semaphore");
        }
    }

    if((sem_ovenOUT = sem_open(sem_ovenOUT_name, O_CREAT | O_RDWR | O_EXCL, PERMS, 1)) == SEM_FAILED){
        perror("Cannot create semaphore ovenOUT");
        exit(1);
    }

    if((sem_getvalue(sem_ovenOUT, &semval)) == -1){
        perror("Cannot get value of semaphore");
    }
    printf("semval: %d\n", semval);

    // table semaphores ----------------------------------------------------------------------
    if((sem_tableIN = sem_open(sem_tableIN_name, O_RDWR, PERMS, MAX_TABLE_CAPACITY)) != SEM_FAILED){
        perror("Semaphore exists");
        if((sem_close(sem_tableIN)) == -1){
            perror("Cannot close tableIN semaphore");
        }
        if((sem_unlink(sem_tableIN_name)) == -1){
            perror("Cannot unlink tableIN semaphore");
        }
    }

    if((sem_tableIN = sem_open(sem_tableIN_name, O_CREAT | O_RDWR | O_EXCL, PERMS, MAX_OVEN_CAPACITY)) == SEM_FAILED){
        perror("Cannot create semaphore tableIN");
        exit(1);
    }

    if((sem_getvalue(sem_tableIN, &semval)) == -1){
        perror("Cannot get value of semaphore");
    }
    printf("semval: %d\n", semval);


    if((sem_tableOUT = sem_open(sem_tableOUT_name, O_RDWR, PERMS, 0)) != SEM_FAILED){
        perror("Semaphore exists");
        if((sem_close(sem_tableOUT)) == -1){
            perror("Cannot close tableOUT semaphore");
        }
        if((sem_unlink(sem_tableOUT_name)) == -1){
            perror("Cannot unlink tableOUT semaphore");
        }
    }

    if((sem_tableOUT = sem_open(sem_tableOUT_name, O_CREAT | O_RDWR | O_EXCL, PERMS, 0)) == SEM_FAILED){
        perror("Cannot create semaphore ovenIN");
        exit(1);
    }

    if((sem_getvalue(sem_tableOUT, &semval)) == -1){
        perror("Cannot get value of semaphore");
    }
    printf("semval: %d\n", semval);

    if((sem_table_update = sem_open(sem_table_update_name, O_CREAT | O_RDWR | O_EXCL, PERMS, 1)) == SEM_FAILED){
        perror("Cannot create semaphore update_table");
        exit(1);
    }

    if((sem_getvalue(sem_table_update, &semval)) == -1){
        perror("Cannot get value of update_table semaphore");
    }
    printf("semval: %d\n", semval);

    // oven memory -------------------------------------------------------------------------------
    if((oven_desc = shm_open(mem_oven_name, O_CREAT | O_RDWR, PERMS)) == -1){
        perror("Cannot create shared memory for oven");
        exit(2);
    }

    if((ftruncate(oven_desc, sizeof(struct oven))) == -1){
        perror("Cannot truncate shared memory for oven");
        exit(2);
    }

    // dolaczenie pamieci do przestrzeni adresowej

    if((oven_mem = mmap(NULL, sizeof(struct oven), PROT_READ | PROT_WRITE, MAP_SHARED, oven_desc, 0)) == (struct oven *) -1) {
        perror("Cannot attach shared memory");
    }
    oven_mem->n = 0;
    for(int i=0; i<MAX_OVEN_CAPACITY; i++){
        oven_mem->content[i] = 10;
    }

    // table memory -------------------------------------------------------------------------------
    if((table_desc = shm_open(mem_table_name, O_CREAT | O_RDWR, PERMS)) == -1){
        perror("Cannot create shared memory for table");
        exit(2);
    }

    if((ftruncate(table_desc, sizeof(struct table))) == -1){
        perror("Cannot truncate shared memory for oven");
        exit(2);
    }

    // dolaczenie pamieci do przestrzeni adresowej
    if((table_mem = mmap(NULL, sizeof(struct table), PROT_READ | PROT_WRITE, MAP_SHARED, table_desc, 0)) == (struct table *) -1) {
        perror("Cannot attach shared memory");
    }
    table_mem->n = 0;
    for(int i=0; i<MAX_OVEN_CAPACITY; i++){
        table_mem->content[i] = 10;
    }

    pid_t childPID;
    for(int i=0; i<n_cooks; i++){
        childPID = fork();
        if(childPID == 0) { // dziecko
            int res = execl("./cook", "cook", sem_ovenIN_name, sem_ovenOUT_name, sem_tableIN_name, sem_tableOUT_name, sem_table_update_name, mem_oven_name, mem_table_name, NULL);
            if (res == -1) {
                perror("Error with execl");
                exit(10);
            }
            exit(0);
        }
        else{
            children[i] = childPID;
            N++;
        }
    }

    for(int i=0; i<n_delivers; i++){
        childPID = fork();
        if(childPID == 0) { // dziecko
            int res = execl("./deliveryman", "deliveryman", sem_tableIN_name, sem_tableOUT_name, sem_table_update_name, mem_table_name, NULL);
            if (res == -1) {
                perror("Error with execl");
                exit(10);
            }
            exit(0);
        }
        else{
            children[n_cooks+i] = childPID;
            N++;
        }
    }

    while(wait(NULL) > 0) {};

    return 0;
}
