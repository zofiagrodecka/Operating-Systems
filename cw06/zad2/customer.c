#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <limits.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include "utils.h"

mqd_t queue_des;
int server_queue_des;
char *queue_name;
char *server_queue_name;
struct mq_attr attributes;
char strChatID[MAX_CHATID_SIZE];
int myChatID;

void remove_queue(void){
    printf("My queue %s, des: %d\n", queue_name, queue_des);

    if(mq_close(queue_des) == -1){
        perror("Cannot close queue");
        exit(100);
    }

    if(mq_close(server_queue_des) == -1){
        perror("Cannot close queue");
        exit(100000);
    }

    if(mq_unlink(queue_name) == -1){
        perror("Cannot delete queue");
        exit(101);
    }
    free(queue_name);
    free(server_queue_name);
}

void sigint_handler(int signum){
    char message_to_server[MAX_MSG_LENGTH];
    char with_chatid[MAX_MSG_LENGTH];
    sprintf(with_chatid, "%d", myChatID);
    strcat(with_chatid, " ");
    strcat(with_chatid, "STOP");

    int type = STOP_TYPE;
    strcpy(message_to_server, with_chatid);

    if(signum == SIGINT){
        if(mq_send(server_queue_des, message_to_server, MAX_MSG_LENGTH, type) == -1){
            perror("Error while sending");
            exit(10);
        }
    }
    exit(130);
}

int inputAvailable(void){
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
    return (FD_ISSET(0, &fds));
}

void enterChatMode(char *chatter_q_name){
    printf("----- CHAT MODE ------\n");
    int chatter_queue_des;
    if((chatter_queue_des = mq_open(chatter_q_name, O_WRONLY | O_EXCL, PERMS, &attributes)) == -1) { // check if exists
        perror("Such queue does not exist");
        exit(10);
    }
    int disconnect = 0;
    int partner_disconnected = 0;
    char msg[MAX_MSG_LENGTH];
    char message[MAX_MSG_LENGTH];
    char to_be_received[MAX_MSG_LENGTH];
    unsigned int *priority = calloc(1, sizeof(unsigned int));

    struct timespec timespec;
    clock_gettime(CLOCK_REALTIME, &timespec);
    timespec.tv_nsec += 5000;
    int sent_type;

    while(!disconnect){
        while (!inputAvailable()) {
            if(mq_timedreceive(queue_des, to_be_received, MAX_MSG_LENGTH+10, priority, &timespec) != -1){ // dostalem wiadomosc
                if(*priority == DISC_TYPE){
                    printf("%s\n", to_be_received);
                    partner_disconnected = 1; // automatycznie rozlacza
                    disconnect = 1;
                    break;
                }
                else if(*priority == STOP_TYPE){
                    printf("%s\n", to_be_received);
                    raise(SIGINT);
                }
                else{
                    printf("PARTNER: %s\n", to_be_received);
                }
            }
        }

        if(! partner_disconnected){
            fgets(msg, MAX_MSG_LENGTH, stdin);
            msg[strlen(msg)-1] = 0;

            if(strcmp(msg, "DISCONNECT") == 0){
                sent_type = DISC_TYPE;
                strcpy(message, "Your partner has disconnected");
                disconnect = 1;
            }
            else{
                sent_type = CHAT_TYPE;
                strcpy(message, msg);
            }

            if(mq_send(chatter_queue_des, message, MAX_MSG_LENGTH, sent_type) == -1){
                perror("Error while sending");
                continue;
            }
        }
    }
    free(priority);
}



int main() {
    printf("PID: %d, Hello, as customer!\n", getpid());
    signal(SIGINT, sigint_handler);
    atexit(remove_queue);
    attributes.mq_curmsgs = 0;
    attributes.mq_flags = 0;
    attributes.mq_maxmsg = MAX_N_MSGS;
    attributes.mq_msgsize = MAX_MSG_LENGTH;

    queue_name = get_customer_queue_name();
    server_queue_name = get_server_queue_name();

    if((queue_des = mq_open(queue_name, O_RDWR | O_CREAT | O_EXCL, PERMS, &attributes)) == -1){
        perror("Error with opening a queue");
        exit(5);
    }
    printf("My queue %s, des: %d\n", queue_name, queue_des);

    if((server_queue_des = mq_open(server_queue_name, O_RDWR |  O_EXCL, PERMS, &attributes)) == -1){
        perror("Error with opening a server queue");
        exit(5);
    }

    printf("server: %s, des: %d\n", server_queue_name, server_queue_des);

    char message_to_server[MAX_QNAME_LENGTH];
    strcpy(message_to_server, queue_name);

    if(mq_send(server_queue_des, message_to_server, MAX_QNAME_LENGTH, INIT_TYPE) == -1){
        perror("Error while sending");
        exit(4);
    }
    printf("CUSTOMER: message sent: %s\n", message_to_server);

    char message_from_server[MAX_MSG_LENGTH];
    unsigned int *priority = calloc(1, sizeof(unsigned int));
    if(mq_receive(queue_des, message_from_server, MAX_MSG_LENGTH+10, priority) == -1) {
        perror("Error while receiving a message");
    }

    printf("My ChatID: %s\n", message_from_server);

    myChatID = (int)strtol(message_from_server, NULL, 10);
    strcpy(strChatID, message_from_server);

    // WYSYLANIE ZLECEN ZE STANDARDOWEGO WYJSCIA
    char order_type[MAX_TYPE_LENGTH];
    char confirmation[MAX_MSG_LENGTH];
    char chat_invitation[MAX_MSG_LENGTH];

    char chatid_added[MAX_MSG_LENGTH];
    int chatter_key;
    int quitting_chat = 0;
    int connecting = 0;
    int end = 0;
    struct timespec timespec;
    clock_gettime(CLOCK_REALTIME, &timespec);
    timespec.tv_nsec += 5000;

    while(!end) {
        if(!connecting){
            printf("Waiting for input...\n");
        }
        connecting = 0;
        chat_invitation[0] = 0;
        quitting_chat = 0;
        while (!inputAvailable()) {
            if(mq_timedreceive(queue_des, chat_invitation, MAX_MSG_LENGTH+10, priority, &timespec) != -1) {
                printf("%s\n", confirmation);

                if (strcmp(chat_invitation, "\0") != 0) {

                    if(*priority == STOP_TYPE){
                        printf("%s\n", chat_invitation);
                        raise(SIGINT);
                    }
                    else {
                    enterChatMode(chat_invitation);
                    chat_invitation[0] = 0;

                    strcpy(order_type, "DISCONNECT");
                    quitting_chat = 1;
                    break; // pozniej obsluguje DISCONNECT z serverem
                    }
                }
            }
        }

        if(!quitting_chat) {
            fgets(order_type, MAX_TYPE_LENGTH + 1, stdin);
            order_type[strlen(order_type) - 1] = 0;
        }

        // dodawanie chatid do wiadomosci
        sprintf(chatid_added, "%d", myChatID);
        strcat(chatid_added, " ");
        strcat(chatid_added, order_type);

        if((*priority = stringToType(order_type)) == -1){
            continue;
        }
        strcpy(message_to_server, chatid_added);

        if(mq_send(server_queue_des, message_to_server, MAX_TYPE_LENGTH, *priority) == -1){
            perror("Error while sending");
            exit(4);
        }
        printf("CUSTOMER: message sent: %s\n", message_to_server);

        if (strcmp(order_type, "STOP") == 0) {
            printf("Finished :)\n");
            end = 1;
        }
        else if(strcmp(order_type, "CONNECT") == 0) {
            connecting = 1;
        }
        else {
            // oczekiwanie na komunikat od serwera o wykonaniu zlecenia
            if(mq_receive(queue_des, confirmation, MAX_MSG_LENGTH+10, priority) == -1) {
                perror("Error while receiving a message");
            }
            printf("%s\n", confirmation);
        }
    }

    free(priority);
    return 0;
}
