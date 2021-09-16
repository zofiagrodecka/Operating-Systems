#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

#define END_VAL 3
#define N_ELVES 10
#define N_REINDEERS 9
#define N_PROBLEMS 3
#define DELIVERY_TIME 4
#define SOLVING_PROBLEM_TIME 2

int n_problematic_elves = 0;
int n_reindeers_back = 0;
int n_delivered_gifts = 0;
pthread_t threads[N_ELVES + N_REINDEERS + 1];

pthread_mutex_t elf_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
sem_t elf_sem[N_PROBLEMS];
sem_t elf_main_guard;
pthread_t elves[N_PROBLEMS];

pthread_mutex_t reindeer_mutex = PTHREAD_MUTEX_INITIALIZER;
sem_t reindeer_sem[N_REINDEERS];

pthread_mutex_t santa_mutex = PTHREAD_MUTEX_INITIALIZER;

void *santa(void *args){

    while(1) {
        pthread_mutex_lock(&santa_mutex);
        while (n_problematic_elves < N_PROBLEMS && n_reindeers_back < N_REINDEERS) {
            pthread_cond_wait(&cond, &santa_mutex);
        }
        printf("MIKOLAJ: Budze sie...\n");

        // po obudzeniu
        pthread_mutex_lock(&reindeer_mutex);
        if (n_reindeers_back == N_REINDEERS) {
            for(int i=0; i<N_REINDEERS; i++){
                if((sem_post(&reindeer_sem[i])) != 0){
                    perror("Santa cannot increase reindeer semaphore");
                }
            }

            printf("MIKOLAJ: Dostarczam zabawki\n");
            sleep(DELIVERY_TIME);
            n_reindeers_back = 0;
            n_delivered_gifts++;
            if (n_delivered_gifts == END_VAL) {
                for (int i = 0; i < N_ELVES + N_REINDEERS; i++) { // Mikolaj ma indeks ostatni zawsze
                    pthread_cancel(threads[i]);
                }

                return 0;
            }
        }
        pthread_mutex_unlock(&reindeer_mutex);

        pthread_mutex_lock(&elf_mutex);
        if (n_problematic_elves == N_PROBLEMS) {
            for(int i=0; i<N_PROBLEMS; i++){
                if((sem_post(&elf_sem[i])) != 0){
                    perror("Santa cannot increase elf semaphore");
                }
            }

            printf("MIKOLAJ: Rozwiazuje problemy elfow: %lu, %lu, %lu\n", elves[0], elves[1], elves[2]);
            sleep(SOLVING_PROBLEM_TIME);
            n_problematic_elves -= 3;
            printf("MIKOLAJ: Rozwiazalem problemy elfow. Zostalo %d z problemami\n", n_problematic_elves);

            for(int i=0; i<N_PROBLEMS; i++) {
                if ((sem_post(&elf_main_guard)) != 0) {
                    perror("Santa cannot increase elf guard semaphore");
                }
            }
        }
        pthread_mutex_unlock(&elf_mutex);

        pthread_mutex_unlock(&santa_mutex);
        printf("MIKOLAJ: Zasypiam...\n");
    }
}

void *elf(void *args){

    if((pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL)) != 0){
        perror("Error while setting cancel type in elf");
        return (void *) 1;
    }
    if((pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL)) != 0){
        perror("Error while enabling elf cancellation");
        return (void *) 1;
    }

    pthread_t id = pthread_self();
    int work_time;
    int my_ind;
    while(1){
        work_time = rand()%(5-2+1) + 2;
        printf("ELF: %lu, Pracuje: %d s\n", id, work_time);
        sleep(work_time);

        if(n_problematic_elves >= N_PROBLEMS){
            printf("ELF: %lu czeka na powrot elfow\n", id);
        }

        sem_wait(&elf_main_guard);

        pthread_mutex_lock(&elf_mutex);
        my_ind = n_problematic_elves;
        elves[n_problematic_elves] = id;
        n_problematic_elves++;
        printf("ELF: %lu, czeka %d elfow na Mikolaja (wlacznie ze mna). Podchodze do semafora: %d\n", id, n_problematic_elves, my_ind);

        if(n_problematic_elves == N_PROBLEMS){
            printf("ELF: %lu, Wybudzam Mikolaja\n", id);
            pthread_cond_broadcast(&cond);
        }
        pthread_mutex_unlock(&elf_mutex);

        sem_wait(&elf_sem[my_ind]); // czekam az mnie Mikolaj wpusci
        printf("ELF: %lu, Mikolaj rozwiazuje problem\n", id);
        sleep(SOLVING_PROBLEM_TIME);
   }

}

void *reindeer(void *args){

    if((pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL)) != 0){
        perror("Error while setting cancel type in reindeer");
        return (void *) 1;
    }
    if((pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL)) != 0){
        perror("Error while enabling reindeer cancellation");
        return (void *) 1;
    }

    pthread_t id = pthread_self();
    int work_time;
    int my_ind;
    while(1){
        work_time = rand()%(10-5+1) + 5;
        printf("RENIFER: %lu, Jade na wakacje: %d s\n", id, work_time);
        sleep(work_time);

        pthread_mutex_lock(&reindeer_mutex);
        my_ind = n_reindeers_back;
        n_reindeers_back++;
        printf("RENIFER: czeka %d reniferow na Mikolaja (wlacznie ze mna)\n", n_reindeers_back);

        if(n_reindeers_back == N_REINDEERS){
            printf("RENIFER: %lu, Wybudzam Mikolaja\n", id);
            pthread_cond_broadcast(&cond);
        }
        pthread_mutex_unlock(&reindeer_mutex);

        sem_wait(&reindeer_sem[my_ind]); // czekam az mnie Mikolaj wpusci
        sleep(DELIVERY_TIME);
    }
}

int main() {

    srand(time(0));

    if((pthread_mutex_init(&elf_mutex, NULL)) != 0){
        perror("Cannot initialize elf_mutex");
        exit(1);
    }

    if((pthread_mutex_init(&reindeer_mutex, NULL)) != 0){
        perror("Cannot initialize reindeer_mutex");
        exit(1);
    }

    if((pthread_mutex_init(&santa_mutex, NULL)) != 0){
        perror("Cannot initialize santa_mutex");
        exit(1);
    }

    if((sem_init(&elf_main_guard, 0, 3)) == -1){
        perror("Error while initializing guard semaphore");
        exit(1);
    }

    for(int i=0; i<N_PROBLEMS; i++){
        if((sem_init(&elf_sem[i], 0, 0)) == -1){
            perror("Error while initializing elf semaphore");
            exit(1);
        }
    }

    for(int i=0; i<N_REINDEERS; i++){
        if((sem_init(&reindeer_sem[i], 0, 0)) == -1){
            perror("Error while initializing reindeer semaphore");
            exit(1);
        }
    }

    int ind = 0;
    for(int i=0; i<N_ELVES; i++){

        if((pthread_create(&threads[ind], NULL, elf, NULL)) != 0){
            perror("Error while creating elf thread");
            exit(1);
        }
        ind ++;
    }

    for(int i=0; i<N_REINDEERS; i++){

        if((pthread_create(&threads[ind], NULL, reindeer, NULL)) != 0){
            perror("Error while creating reindeer thread");
            exit(1);
        }
        ind ++;
    }

    // Mikolaj
    if((pthread_create(&threads[ind], NULL, santa, NULL)) != 0){
        perror("Error while creating santa thread");
        exit(1);
    }
    ind ++;

    for(int i=0; i<ind; i++){

        if((pthread_join(threads[i], NULL)) != 0){
            perror("Error while joining threads");
            exit(2);
        }
    }

    if((sem_destroy(&elf_main_guard)) == -1){
        perror("Cannot destroy guard semaphore");
        exit(1);
    }

    for(int i=0; i<N_PROBLEMS; i++){
        if((sem_destroy(&elf_sem[i])) == -1){
            perror("Cannot destroy elf semaphore");
            exit(1);
        }
    }

    for(int i=0; i<N_REINDEERS; i++){
        if((sem_destroy(&reindeer_sem[i])) == -1){
            perror("Cannot destroy reindeer semaphore");
            exit(1);
        }
    }

    return 0;
}
