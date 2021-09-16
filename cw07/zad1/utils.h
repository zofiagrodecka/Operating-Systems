#ifndef ZAD1_UTILS_H
#define ZAD1_UTILS_H

#define PERMS 0666
#define MAX_OVEN_CAPACITY 5
#define MAX_TABLE_CAPACITY 5
#define MAX_PIZZA_TYPE 9
#define MAX_KEY_LENGTH 15
#define BAKING_TIME 5
#define DELIVERY_TIME 5
#define MAX_N_WORKERS 20

union semun {
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
};

struct oven{
    int content[MAX_OVEN_CAPACITY];
    int n; // aktualna ilosc pizz
};

struct table{
    int content[MAX_TABLE_CAPACITY];
    int n; // aktualna ilosc pizz
};

void decrease_semaphore(int semid);
void increase_semaphore(int semid);
int add_pizza_to_oven(int pizza_type, struct oven *pmem);
void add_pizza_to_table(int pizza_type, struct table *pmem);
void get_pizza_from_oven(int pizza_index, struct oven *pmem);
int get_pizza_from_table(struct table *pmem);
#endif //ZAD1_UTILS_H
