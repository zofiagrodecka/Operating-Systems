#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "utils.h"

struct cust{
    int queueID;
    int key;
    int isActive;
};

int queueID;
int actual_n_customers = 0;
int deleted_n_customers = 0;
int end = 0;
struct cust customers[MAX_CUSTOMERS_NUMBER]; // tworze tablice klientow

struct cust emptyCust;

void remove_queue(void){
    msgctl(queueID, IPC_RMID, NULL);
}

void sigint_handler(int signum){
    struct message stop_message;
    stop_message.mtype = STOP_TYPE;
    strcpy(stop_message.mtext, "Server stopped working.");

    struct message order_type;
    int customerChatID;
    char *pstr;

    if(signum == SIGINT){
        for(int i=0; i<actual_n_customers; i++){
            if(customers[i].queueID != -1) {
                if(msgsnd(customers[i].queueID, &stop_message, MAX_MSG_LENGTH, 0) == -1){
                    perror("Error while sending");
                    exit(131);
                }
                printf("Message sent: %s\n", stop_message.mtext);
            }
        }

        while(!end){
            if (msgrcv(queueID, &order_type, MAX_TYPE_LENGTH, STOP_TYPE, MSG_NOERROR) == -1) {
                perror("Error while receiving");
                exit(131);
            }
            printf("SERVER: Mesage received: %s\n", order_type.mtext);

            pstr = strtok(order_type.mtext, " ");
            customerChatID = (int)strtol(pstr, NULL, 10);

            if(order_type.mtype == STOP_TYPE){
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
    exit(130);
}

int main() {
    printf("PID: %d, Hello, as server!\n", getpid());

    signal(SIGINT, sigint_handler);

    emptyCust.isActive = 0;
    emptyCust.key = 0;
    emptyCust.queueID = -1; // bo get_customer_key nigdy nie wygeneruje -1 bo to blad fftoka

    key_t key = get_system_key(); // zawsze taki sam

    if((queueID = msgget(key, 0)) > 0) { // check if exists
        perror("SERVER: Deleting existing queue");
        msgctl(queueID, IPC_RMID, NULL);
    }

    if((queueID = msgget(key, PERMS | IPC_CREAT | IPC_EXCL)) < 0){ // blad EEXIST jesli taka kolejka juz istnieje
        perror("Error creating system queue");
        exit(3);
    }

    printf("SERVER: my queue: %d\n", queueID);

    // OCZEKIWANIE NA ZLECENIE ---------------------------------------------------------------------------------
    char *pstr;
    int customerChatID;
    struct message order_type;
    struct message confirmation;
    int customer_queueID;
    struct cust new_customer;
    char cust_chatID[MAX_CHATID_SIZE];
    int stop = 0;
    char active_users_list[MAX_MSG_LENGTH];
    char str_id[MAX_CHATID_SIZE];
    char str_active[3];
    int invitedID;
    char cust_key[MAX_KEY_LENGTH];
    char invited_key[MAX_KEY_LENGTH];

    while(!end){
        stop = 0;

        if (msgrcv(queueID, &order_type, MAX_TYPE_LENGTH, MAX_TYPE*(-1), MSG_NOERROR) == -1) {
            perror("Error while receiving");
            exit(4);
        }
        printf("SERVER: Mesage received: %s\n", order_type.mtext);
        confirmation.mtype = order_type.mtype;

        if(order_type.mtype == INIT_TYPE){
            key_t customerKEY = (int)strtol(order_type.mtext, NULL, 10);

            printf("SERVER: Customer key: %d\n", customerKEY);

            if((customer_queueID = msgget(customerKEY, 0)) < 0){ // msgflg == 0 - uzyskanie id juz stworzonej kolejki
                perror("Error finding customer queue");
                exit(3);
            }
            printf("Customer queue: %d\n", customer_queueID);

            // dodawanie klienta do tablicy
            new_customer.queueID = customer_queueID;
            new_customer.key = customerKEY;
            new_customer.isActive = 1;
            customers[actual_n_customers] = new_customer;
            customerChatID = actual_n_customers; // zeby wyslal potwierdzenie do dobrego klienta

            sprintf(cust_chatID, "%d", actual_n_customers);
            strcpy(confirmation.mtext, cust_chatID);
            actual_n_customers++;
        }
        else{
            pstr = strtok(order_type.mtext, " ");
            customerChatID = (int)strtol(pstr, NULL, 10);

            pstr = strtok(NULL, " ");
            printf("SERVER: Mesage order type: %s, customer id: %d\n", pstr, customerChatID);

            if(strcmp(pstr, "STOP") == 0){
                stop = 1;
                customers[customerChatID] = emptyCust;
                deleted_n_customers++;
                if(deleted_n_customers >= actual_n_customers){
                    end = 1;
                    printf("Ending work...\n");
                }
            }
            else if(strcmp(pstr, "LIST") == 0){
                strcpy(active_users_list, "\nCustomerID isActive\n");
                for(int i=0; i<actual_n_customers; i++){
                    if(customers[i].queueID != -1) {
                        sprintf(str_id, "%d", i);
                        sprintf(str_active, "%d\n", customers[i].isActive);
                        strcat(active_users_list, str_id);
                        strcat(active_users_list, "          ");
                        strcat(active_users_list, str_active);
                    }
                }
                strcpy(confirmation.mtext, active_users_list);
            }
            else if(strcmp(pstr, "DISCONNECT") == 0){
                strcpy(confirmation.mtext, "Disconnecting...");
                customers[customerChatID].isActive = 1;
            }
            else if(strcmp(pstr, "CONNECT") == 0){
                pstr = strtok(NULL, " "); // teraz ma id klienta z ktorym mam polaczyc
                if(!isNumber(pstr)){
                    perror("Argument after CONNECT is not a number");
                    continue;
                }
                invitedID = (int)strtol(pstr, NULL, 10);
                sprintf(cust_key, "%d", customers[customerChatID].key);
                sprintf(invited_key, "%d", customers[invitedID].key);

                // sending to the invited one
                strcpy(confirmation.mtext, cust_key);
                printf("After strcpy: %s\n", confirmation.mtext);
                if (msgsnd(customers[invitedID].queueID, &confirmation, MAX_MSG_LENGTH, 0) == -1) {
                    perror("Error while sending");
                }
                printf("SERVER: message: %s, sent to: %d\n", confirmation.mtext, customers[invitedID].queueID);

                // sending to the one with order
                strcpy(confirmation.mtext, invited_key);

                customers[customerChatID].isActive = 0;
                customers[invitedID].isActive = 0;
            }
            else{
                perror("Wrong order type");
                continue;
            }
        }

        if(!end && !stop) {
            if (msgsnd(customers[customerChatID].queueID, &confirmation, MAX_MSG_LENGTH, 0) == -1) {
                perror("Error while sending");
                exit(4);
            }
            printf("SERVER: message: %s, sent to: %d\n", confirmation.mtext, customers[customerChatID].queueID);
        }
    }

    atexit(remove_queue);
    return 0;
}
