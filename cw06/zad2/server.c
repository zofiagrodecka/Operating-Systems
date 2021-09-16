#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <limits.h>
#include <unistd.h>
#include <signal.h>
#include "utils.h"

struct cust{
    mqd_t queue_desc;
    char queue_name[MAX_QNAME_LENGTH];
    int isActive;
};

mqd_t queue_des;
char *queue_name;
int actual_n_customers = 0;
int deleted_n_customers = 0;
int end = 0;
struct cust customers[MAX_CUSTOMERS_NUMBER]; // tworze tablice klientow

struct cust emptyCust;

void remove_queue(void){
    if(mq_close(queue_des) == -1){
        perror("Cannot close queue");
        exit(100);
    }
    if(mq_unlink(queue_name) == -1) {
        perror("Cannot delete queue");
        exit(101);
    }
}

void sigint_handler(int signum){
    char stop_message[MAX_MSG_LENGTH];
    int type = STOP_TYPE;
    strcpy(stop_message, "Server stopped working.");

    char order_type[MAX_MSG_LENGTH];
    int customerChatID;
    char *pstr;
    unsigned int *priority = calloc(1, sizeof(unsigned int));

    if(signum == SIGINT){
        for(int i=0; i<actual_n_customers; i++){
            if(strcmp(customers[i].queue_name, "/00") != 0) {
                if(mq_send(customers[i].queue_desc, stop_message, MAX_MSG_LENGTH, type) == -1){
                    perror("Error while sending");
                    exit(131);
                }
                printf("Message sent: %s\n", stop_message);
            }
        }

        while(!end){
            if(mq_receive(queue_des, order_type, MAX_MSG_LENGTH+10, priority) == -1) {
                perror("Error while receiving a message");
                exit(131);
            }
            printf("SERVER: Mesage received: %s\n", order_type);

            pstr = strtok(order_type, " ");
            customerChatID = (int)strtol(pstr, NULL, 10);

            if(*priority == STOP_TYPE){
                customers[customerChatID] = emptyCust;
                deleted_n_customers++;
                if(deleted_n_customers >= actual_n_customers){
                    end = 1;
                    printf("Ending work...\n");
                }
            }
        }

        remove_queue();
    }
    free(priority);
    exit(130);
}

int main() {

    printf("PID: %d, Hello, as server!\n", getpid());
    signal(SIGINT, sigint_handler);

    emptyCust.isActive = 0;
    emptyCust.queue_desc = 0; // STDIN
    strcpy(emptyCust.queue_name, "/00"); // proces z pid==0 to demon


    struct mq_attr attributes;
    attributes.mq_curmsgs = 0;
    attributes.mq_flags = 0;
    attributes.mq_maxmsg = MAX_N_MSGS;
    attributes.mq_msgsize = MAX_MSG_LENGTH;


    queue_name = get_server_queue_name();
    if((queue_des = mq_open(queue_name, O_RDWR | O_EXCL, PERMS, &attributes)) > 0){
        perror("Deleting existing queue");
        if(mq_close(queue_des) == -1){
            perror("Cannot close queue");
            exit(150);
        }
        if(mq_unlink(queue_name) == -1) {
            perror("Cannot delete queue");
            exit(151);
        }
    }

    if((queue_des = mq_open(queue_name, O_RDWR | O_CREAT | O_EXCL, PERMS, &attributes)) == -1){
        perror("Error with opening system queue");
    }

    printf("SERVER: my queue: %s, my desc: %d\n", queue_name, queue_des);

    struct cust new_customer;
    char order_type[MAX_TYPE_LENGTH];
    order_type[0] = 0;
    unsigned int *priority = calloc(1, sizeof(unsigned int));
    int customerChatID;
    char str_cust_chatID[MAX_CHATID_SIZE];
    char confirmation[MAX_MSG_LENGTH];
    int stop = 0;
    char customer_queue_name[MAX_QNAME_LENGTH];
    char active_users_list[MAX_MSG_LENGTH];
    char str_id[MAX_CHATID_SIZE];
    char str_active[3];
    char *pstr;
    int invitedID;
    char invited_q_name[MAX_QNAME_LENGTH];

    while(!end) {
        stop = 0;
        printf("Waiting for message...\n");
        if (mq_receive(queue_des, order_type, MAX_MSG_LENGTH + 10, priority) == -1) {
            perror("Error while receiving a message");
            continue;
        }

        printf("SERVER: Mesage received: %s, Priority: %d\n", order_type, *priority);

        if(*priority == INIT_TYPE) {
            strcpy(customer_queue_name, order_type);

            printf("SERVER: Customer name: %s\n", customer_queue_name);
            mqd_t customer_queue_desc;
            if ((customer_queue_desc = mq_open(customer_queue_name, O_RDWR | O_EXCL, PERMS, &attributes)) == -1) {
                perror("Error finding customer queue");
                exit(10);
            }
            printf("Customer queue desc: %d\n", customer_queue_desc);

            // dodawanie klienta do tablicy
            new_customer.queue_desc = customer_queue_desc;
            strcpy(new_customer.queue_name, customer_queue_name);
            new_customer.isActive = 1;
            customers[actual_n_customers] = new_customer;
            customerChatID = actual_n_customers; // zeby wyslal potwierdzenie do dobrego klienta

            sprintf(str_cust_chatID, "%d", actual_n_customers);
            strcpy(confirmation, str_cust_chatID);
            actual_n_customers++;
        }
        else {
            pstr = strtok(order_type, " ");
            customerChatID = (int) strtol(pstr, NULL, 10);

            pstr = strtok(NULL, " ");
            printf("SERVER: Mesage order type: %s, customer id: %d\n", pstr, customerChatID);

            if (*priority == STOP_TYPE) {
                stop = 1;

                if(mq_close(customers[customerChatID].queue_desc) == -1){
                    perror("Cannot close customer queue");
                    exit(100);
                }
                printf("Closed customer queue\n");
                customers[customerChatID] = emptyCust;
                deleted_n_customers++;

                if (deleted_n_customers >= actual_n_customers) {
                    end = 1;
                    printf("Ending work...\n");
                }
            }
            else if(*priority == LIST_TYPE){
                strcpy(active_users_list, "\nCustomerID isActive\n");
                for(int i=0; i<actual_n_customers; i++){
                    if(strcmp(customers[i].queue_name, "/00") != 0) {
                        sprintf(str_id, "%d", i);
                        sprintf(str_active, "%d\n", customers[i].isActive);
                        strcat(active_users_list, str_id);
                        strcat(active_users_list, "          ");
                        strcat(active_users_list, str_active);
                    }
                }
                strcpy(confirmation, active_users_list);
            }
            else if(*priority == DISC_TYPE){
                strcpy(confirmation, "Disconnecting...");
                customers[customerChatID].isActive = 1;
            }
            else if(*priority == CHAT_TYPE){
                pstr = strtok(NULL, " "); // teraz ma id klienta z ktorym mam polaczyc

                invitedID = (int)strtol(pstr, NULL, 10);

                // sending to the invited one
                strcpy(confirmation, customers[customerChatID].queue_name);
                if (mq_send(customers[invitedID].queue_desc, confirmation, MAX_MSG_LENGTH, *priority) == -1) {
                    perror("Error while sending");
                }
                printf("SERVER: message: %s, sent to desc: %d\n", confirmation, customers[invitedID].queue_desc);

                // sending to the one with order
                strcpy(confirmation, customers[invitedID].queue_name);

                customers[customerChatID].isActive = 0;
                customers[invitedID].isActive = 0;
            }
            else{
                perror("Wrong order type");
                continue;
            }
        }

        if(!end && !stop) {
            if (mq_send(customers[customerChatID].queue_desc, confirmation, MAX_MSG_LENGTH, *priority) == -1) {
                perror("Error while sending");
                exit(4);
            }
            printf("SERVER: message: %s, sent to desc: %d\n", confirmation, customers[customerChatID].queue_desc);
        }
    }

    free(priority);
    atexit(remove_queue);
    return 0;
}
