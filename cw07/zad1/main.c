#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <string.h>
#include "utils.h"

pid_t children[MAX_N_WORKERS];
int N=0;

int sem_ovenIN_id;
int sem_ovenOUT_id;
int sem_tableIN_id;
int sem_tableOUT_id;
int sem_table_update_id;
struct oven *oven_mem;
struct table *table_mem;
int ovenID;
int tableID;

void sigint_handler(int signum){
    if(signum == SIGINT){
        for(int i=0; i<N; i++){
            kill(children[i], SIGINT);
        }

        while(wait(NULL) > 0) {};
        printf("Dzieci sie skonczyly\n");

        if((semctl(sem_ovenIN_id, 0, IPC_RMID, 0)) == -1){
            perror("Cannot remove oven semaphore");
        }

        if((semctl(sem_ovenOUT_id, 0, IPC_RMID, 0)) == -1){
            perror("Cannot remove oven semaphore");
        }

        if((semctl(sem_tableIN_id, 0, IPC_RMID, 0)) == -1){
            perror("Cannot remove table semaphore");
        }

        if((semctl(sem_tableOUT_id, 0, IPC_RMID, 0)) == -1){
            perror("Cannot remove table semaphore");
        }

        if((semctl(sem_table_update_id, 0, IPC_RMID, 0)) == -1){
            perror("Cannot remove table semaphore");
        }

        if(shmdt(oven_mem) == -1){
            perror("Cannot detach oven memory");
        }

        if(shmdt(table_mem) == -1){
            perror("Cannot detach table memory");
        }

        if((shmctl(ovenID, IPC_RMID, NULL)) == -1){
            perror("Cannot delete memory for oven");
        }

        if((shmctl(tableID, IPC_RMID, NULL)) == -1){
            perror("Cannot delete memory for table");
        }
    }
    exit(130);
}

int main(int argc, char *argv[]) {
    if(argc != 3){
        perror("Wrong number of arguments");
        exit(10);
    }

    signal(SIGINT, sigint_handler);

    printf("PID: %d, Hello, World!\n", getpid());
    pid_t parentPID = getpid();

    int n_cooks = (int)strtol(argv[1], NULL, 10);
    int n_delivers = (int)strtol(argv[2], NULL, 10);

    // initialization
    // oven semaphores ----------------------------------------------------------------------
    key_t sem_ovenIN_key = ftok("/home", parentPID - 1);

    if((sem_ovenIN_id = semget(sem_ovenIN_key, 1, PERMS | IPC_CREAT)) == -1){
        perror("Cannot create semaphore");
        exit(1);
    }

    union semun args;
    args.val = MAX_OVEN_CAPACITY; // liczacy

    if((semctl(sem_ovenIN_id, 0, SETVAL, args)) == -1){
        perror("Cannot set value of semaphore");
        exit(1);
    }

    key_t sem_ovenOUT_key = ftok("/home", parentPID - 2);

    if((sem_ovenOUT_id = semget(sem_ovenOUT_key, 1, PERMS | IPC_CREAT)) == -1){
        perror("Cannot create semaphore");
        exit(1);
    }

    args.val = 1; // binarny - odblokowany

    if((semctl(sem_ovenOUT_id, 0, SETVAL, args)) == -1){
        perror("Cannot set value of semaphore");
        exit(1);
    }

    // table semaphore ----------------------------------------------------------------------
    key_t sem_tableIN_key = ftok("/home", parentPID - 3);

    if((sem_tableIN_id = semget(sem_tableIN_key, 1, PERMS | IPC_CREAT)) == -1){
        perror("Cannot create semaphore");
        exit(1);
    }

    args.val = MAX_TABLE_CAPACITY;

    if((semctl(sem_tableIN_id, 0, SETVAL, args)) == -1){
        perror("Cannot set value of semaphore");
        exit(1);
    }

    key_t sem_tableOUT_key = ftok("/home", parentPID - 4);

    if((sem_tableOUT_id = semget(sem_tableOUT_key, 1, PERMS | IPC_CREAT)) == -1){
        perror("Cannot create semaphore");
        exit(1);
    }

    args.val = 0; // dopiero kucharz go podniesie jak wylozy pizze na stol

    if((semctl(sem_tableOUT_id, 0, SETVAL, args)) == -1){
        perror("Cannot set value of semaphore");
        exit(1);
    }

    // table update ----------------------------------------------------------------------
    key_t sem_table_update_key = ftok("/home", parentPID - 5);

    if((sem_table_update_id = semget(sem_table_update_key, 1, PERMS | IPC_CREAT)) == -1){
        perror("Cannot create update semaphore");
        exit(1);
    }

    args.val = 1; // binarny

    if((semctl(sem_table_update_id, 0, SETVAL, args)) == -1){
        perror("Cannot set value of update semaphore");
        exit(1);
    }

    // oven memory -------------------------------------------------------------------------------
    key_t oven_key = ftok("/home", parentPID+1);

    if((ovenID = shmget(oven_key, sizeof(struct oven), PERMS | IPC_CREAT)) == -1){
        perror("Cannot create shared memory for oven");
        exit(2);
    }

    if((oven_mem = shmat(ovenID, NULL, 0)) == (struct oven *) -1) { // dolaczenie pamieci do przestrzeni adresowej
        perror("Cannot attach shared memory");
    }
    oven_mem->n = 0;
    for(int i=0; i<MAX_OVEN_CAPACITY; i++){
        oven_mem->content[i] = 10;
    }

    // table memory -------------------------------------------------------------------------------
    key_t table_key = ftok("/home", parentPID+2);

    if((tableID = shmget(table_key, sizeof(struct table), PERMS | IPC_CREAT)) == -1){
        perror("Cannot create shared memory for table");
        exit(2);
    }

    if((table_mem = shmat(tableID, NULL, 0)) == (struct table *) -1) { // dolaczenie pamieci do przestrzeni adresowej
        perror("Cannot attach shared memory for table");
    }
    table_mem->n = 0;
    for(int i=0; i<MAX_TABLE_CAPACITY; i++){
        table_mem->content[i] = 10;
    }

    char str_sem_ovenIN_key[MAX_KEY_LENGTH] = "\0";
    sprintf(str_sem_ovenIN_key, "%d", sem_ovenIN_key);
    char str_sem_ovenOUT_key[MAX_KEY_LENGTH] = "\0";
    sprintf(str_sem_ovenOUT_key, "%d", sem_ovenOUT_key);
    char str_sem_tableIN_key[MAX_KEY_LENGTH] = "\0";
    sprintf(str_sem_tableIN_key, "%d", sem_tableIN_key);
    char str_sem_tableOUT_key[MAX_KEY_LENGTH] = "\0";
    sprintf(str_sem_tableOUT_key, "%d", sem_tableOUT_key);
    char str_sem_table_update_key[MAX_KEY_LENGTH] = "\0";
    sprintf(str_sem_table_update_key, "%d", sem_table_update_key);

    char str_mem_oven_key[MAX_KEY_LENGTH] = "\0";
    sprintf(str_mem_oven_key, "%d", oven_key);
    char str_mem_table_key[MAX_KEY_LENGTH] = "\0";
    sprintf(str_mem_table_key, "%d", table_key);

    pid_t childPID;
    for(int i=0; i<n_cooks; i++){
        childPID = fork();
        if(childPID == 0) { // dziecko
            int res = execl("./cook", "cook", str_sem_ovenIN_key, str_sem_ovenOUT_key, str_sem_tableIN_key, str_sem_tableOUT_key, str_sem_table_update_key, str_mem_oven_key, str_mem_table_key, NULL);
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
            int res = execl("./deliveryman", "deliveryman", str_sem_tableIN_key, str_sem_tableOUT_key, str_sem_table_update_key, str_mem_table_key, NULL);
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
    printf("Dzieci sie skonczyly\n");

    if((semctl(sem_ovenIN_id, 0, IPC_RMID, 0)) == -1){
        perror("Cannot remove oven semaphore");
    }

    if((semctl(sem_ovenOUT_id, 0, IPC_RMID, 0)) == -1){
        perror("Cannot remove oven semaphore");
    }

    if((semctl(sem_tableIN_id, 0, IPC_RMID, 0)) == -1){
        perror("Cannot remove table semaphore");
    }

    if((semctl(sem_tableOUT_id, 0, IPC_RMID, 0)) == -1){
        perror("Cannot remove table semaphore");
    }

    if(shmdt(oven_mem) == -1){
        perror("Cannot detach oven memory");
    }

    if(shmdt(table_mem) == -1){
        perror("Cannot detach table memory");
    }

    if((shmctl(ovenID, IPC_RMID, NULL)) == -1){
        perror("Cannot delete memory for oven");
    }

    if((shmctl(tableID, IPC_RMID, NULL)) == -1){
        perror("Cannot delete memory for table");
    }

    return 0;
}
