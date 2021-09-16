#ifndef ZAD1_UTILS_H
#define ZAD1_UTILS_H

#define PERMS 0666
#define MAX_CUSTID_LENGTH 20
#define MAX_MSG_LENGTH 1000
#define MAX_TYPE_LENGTH 15
#define MAX_TYPE 5
#define MAX_CUSTOMERS_NUMBER 50
#define MAX_CHATID_SIZE 10
#define MAX_KEY_LENGTH 15

#define CHAT_TYPE 5
#define INIT_TYPE 4
#define LIST_TYPE 3
#define DISC_TYPE 2
#define STOP_TYPE 1

struct message{
    long mtype;
    char mtext[MAX_MSG_LENGTH];
};

key_t get_system_key(void);
key_t generate_customer_key(void);
int stringToType(char *str);
int isNumber(char *string);

#endif //ZAD1_UTILS_H
