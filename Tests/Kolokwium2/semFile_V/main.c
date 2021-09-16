#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <time.h>
#include <sys/wait.h>

#define FILE_NAME "common.txt"
#define SEM_NAME "/kol_sem"

union semun {
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
};


int main(int argc, char** args){

   if(argc !=4){
    printf("Not a suitable number of program parameters\n");
    return(1);
   }

    /**************************************************
    Stworz semafor systemu V
    Ustaw jego wartosc na 1
    ***************************************************/
    key_t key = ftok(SEM_NAME, 1);
    int semid;
    if((semid = semget(key, 1, 0766 | IPC_CREAT)) == -1){
        perror("Cannot create semaphore");
        return 1;
    }

    union semun arg;
    arg.val = 1;
    if((semctl(semid, 0, SETVAL, arg)) == -1){
        perror("Cannot set value");
        return 1;
    }
    
     
     int fd = open(FILE_NAME, O_WRONLY | O_CREAT | O_TRUNC , 0644);
     
     int parentLoopCounter = atoi(args[1]);
     int childLoopCounter = atoi(args[2]);
        
     char buf[50]; // zmienilam wartosc bufora z 20 na 50 poniewaz komunikat zapisywany tu w sprintfie sie nie miesci w calosci wiec bylo stack smashing
     pid_t childPid;
     int max_sleep_time = atoi(args[3]);
     


     if(childPid=fork()){
      int status = 0;
      srand((unsigned)time(0)); 

        while(parentLoopCounter--){
    	    int s = rand()%max_sleep_time+1;
    	    sleep(s);    
            
	    /*****************************************
	    sekcja krytyczna zabezpiecz dostep semaforem
	    **********************************************/
	    struct sembuf sops;
	    sops.sem_num = 0;
	    sops.sem_op = -1;
	    sops.sem_flg = 0;
	    if((semop(semid, &sops, 1)) == -1){
	        perror("Cannot decrease semaphore");
	        return 1;
	    }

        
            sprintf(buf, "Wpis rodzica. Petla %d. Spalem %d\n", parentLoopCounter,s);
	    write(fd, buf, strlen(buf));
	    write(1, buf, strlen(buf));
            
	    /*********************************
	    Koniec sekcji krytycznej
	    **********************************/
            struct sembuf sops2;
            sops2.sem_num = 0;
            sops2.sem_op = 1;
            sops2.sem_flg = 0;
            if((semop(semid, &sops2, 1)) == -1){
                perror("Cannot increase semaphore");
                return 1;
            }

        }
        waitpid(childPid,&status,0);
     }
     else{

	srand((unsigned)time(0)); 
        while(childLoopCounter--){

	    int s = rand()%max_sleep_time+1;
            sleep(s);                
            

	    /*****************************************
	    sekcja krytyczna zabezpiecz dostep semaforem
	    **********************************************/
            struct sembuf sops3;
            sops3.sem_num = 0;
            sops3.sem_op = -1;
            sops3.sem_flg = 0;
            if((semop(semid, &sops3, 1)) == -1){
                perror("Cannot decrease semaphore");
                return 1;
            }

            
            sprintf(buf, "Wpis dziecka. Petla %d. Spalem %d\n", childLoopCounter,s);
            write(fd, buf, strlen(buf));
	    write(1, buf, strlen(buf));

	    /*********************************
	    Koniec sekcji krytycznej
	    **********************************/
            struct sembuf sops4;
            sops4.sem_num = 0;
            sops4.sem_op = 1;
            sops4.sem_flg = 0;
            if((semop(semid, &sops4, 1)) == -1){
                perror("Cannot increase semaphore");
                return 1;
            }

        }
        _exit(0);
     }
     
    /*****************************
    posprzataj semafor
    ******************************/
    if((semctl(semid, 0, IPC_RMID, NULL)) == -1){
        perror("Cannot delete semaphore");
        return 1;
    }

     close(fd);
    return 0;
}
     
        
