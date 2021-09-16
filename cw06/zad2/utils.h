#ifndef ZAD2_UTILS_H
#define ZAD2_UTILS_H

#define PERMS 0666
#define MAX_QNAME_LENGTH 20
#define MAX_MSG_LENGTH 5000
#define MAX_TYPE_LENGTH 15
#define MAX_TYPE 5
#define MAX_CUSTOMERS_NUMBER 50
#define MAX_CHATID_SIZE 10
#define MAX_KEY_LENGTH 15
#define MAX_N_MSGS 10

#define CHAT_TYPE 5
#define INIT_TYPE 4
#define LIST_TYPE 3
#define DISC_TYPE 2
#define STOP_TYPE 1
#define CLOSE_TYPE 6

char *get_customer_queue_name(void);
char *get_server_queue_name(void);
int stringToType(char *str);

#endif //ZAD2_UTILS_H
