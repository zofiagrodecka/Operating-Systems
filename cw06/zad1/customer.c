#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/msg.h>
#include "utils.h"

int queueID;
int server_queueID;
int myChatID;
int end = 0;

void remove_queue(void){
    msgctl(queueID, IPC_RMID, NULL);
}

void sigint_handler(int signum){
    struct message message_to_server;
    char with_chatid[MAX_MSG_LENGTH];
    sprintf(with_chatid, "%d", myChatID);
    strcat(with_chatid, " ");
    strcat(with_chatid, "STOP");

    message_to_server.mtype = STOP_TYPE;
    strcpy(message_to_server.mtext, with_chatid);

    if(signum == SIGINT){
        if(msgsnd(server_queueID, &message_to_server, MAX_TYPE_LENGTH, 0) == -1){
            perror("Error while sending");
            exit(10);
        }
        remove_queue();
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

void enterChatMode(int chatter_key){
    printf("----- CHAT MODE ------\n");
    int chatter_queueID;
    if((chatter_queueID = msgget(chatter_key, 0)) == -1) { // check if exists
        perror("Such queue does not exist");
        exit(5);
    }
    int disconnect = 0;
    int partner_disconnected = 0;
    char msg[MAX_MSG_LENGTH];
    struct message message;
    struct message to_be_received;

    while(!disconnect){
        while (!inputAvailable()) {
            if(msgrcv(queueID, &to_be_received, MAX_MSG_LENGTH, (-1)*MAX_TYPE, MSG_NOERROR | IPC_NOWAIT) != -1) {
                if(to_be_received.mtype == DISC_TYPE){
                    printf("%s\n", to_be_received.mtext);
                    partner_disconnected = 1; // automatycznie rozlacza
                    disconnect = 1;
                    break;
                }
                else if(to_be_received.mtype == STOP_TYPE){
                    printf("%s\n", to_be_received.mtext);
                    raise(SIGINT);
                }
                else{
                    printf("PARTNER: %s\n", to_be_received.mtext);
                }
            }
        }

        if(! partner_disconnected){
            fgets(msg, MAX_MSG_LENGTH, stdin);
            msg[strlen(msg)-1] = 0;

            if(strcmp(msg, "DISCONNECT") == 0){
                message.mtype = DISC_TYPE;
                strcpy(message.mtext, "Your partner has disconnected");
                disconnect = 1;
            }
            else{
                message.mtype = CHAT_TYPE;
                strcpy(message.mtext, msg);
            }

            if (msgsnd(chatter_queueID, &message, MAX_MSG_LENGTH, 0) == -1) {
                perror("Error while sending");
                continue;
            }
        }
    }
}


int main() {
    printf("PID: %d, Hello, as customer!\n", getpid());

    atexit(remove_queue);
    signal(SIGINT, sigint_handler);

    key_t system_key = get_system_key(); // zawsze taki sam

    printf("system key: %d\n", system_key);

    key_t key = generate_customer_key();
    printf("my key: %d\n", key);

    if((queueID = msgget(key, 0)) > 0) { // check if exists
        perror("CUSTOMER: Deleting existing queue");
        msgctl(queueID, IPC_RMID, NULL);
    }

    if((queueID = msgget(key, PERMS | IPC_CREAT | IPC_EXCL)) < 0){
        perror("Error creating customer queue");
        exit(3);
    }

    printf("CUSTOMER: my queue: %d\n", queueID);

    if((server_queueID = msgget(system_key, 0)) < 0){ // msgflg == 0 - uzyskanie id juz stworzonej kolejki
        perror("Error finding system queue");
        exit(3);
    }

    printf("CUSTOMER: server queue: %d\n", server_queueID);

    char customerID[MAX_CUSTID_LENGTH];
    sprintf(customerID, "%d", key);

    struct message message_to_server;
    message_to_server.mtype = INIT_TYPE;
    strcpy(message_to_server.mtext, customerID);

    if(msgsnd(server_queueID, &message_to_server, MAX_CUSTID_LENGTH+1, 0) == -1){
        perror("Error while sending");
        exit(4);
    }
    printf("CUSTOMER: message sent: %s\n", message_to_server.mtext);

    struct message message_from_server;
    if(msgrcv(queueID, &message_from_server, MAX_CUSTID_LENGTH, INIT_TYPE, MSG_NOERROR) == -1){
        perror("Error while receiving");
        exit(4);
    }
    printf("CUSTOMER: Mesage received: %s\n", message_from_server.mtext);

    myChatID = (int)strtol(message_from_server.mtext, NULL, 10);

    printf("My chatID: %d\n", myChatID);

    // WYSYLANIE ZLECEN ZE STANDARDOWEGO WYJSCIA
    char order_type[MAX_TYPE_LENGTH];
    struct message confirmation;
    struct message chat_invitation;

    char chatid_added[MAX_MSG_LENGTH];
    int chatter_key;
    int quitting_chat = 0;
    int connecting = 0;

    while(!end) {
        if(!connecting){
            printf("Waiting for input...\n");
        }
        connecting = 0;
        chat_invitation.mtext[0] = 0;
        quitting_chat = 0;
        while (!inputAvailable()) {
            // check if you have no invitation for a chat
            msgrcv(queueID, &chat_invitation, MAX_MSG_LENGTH, (-1)*MAX_TYPE, MSG_NOERROR | IPC_NOWAIT);

            if(strcmp(chat_invitation.mtext, "\0") != 0){

                if(chat_invitation.mtype == STOP_TYPE){
                    printf("%s\n", chat_invitation.mtext);
                    raise(SIGINT);

                }
                else {
                    chatter_key = (int) strtol(chat_invitation.mtext, NULL, 10);
                    chat_invitation.mtext[0] = 0;
                    enterChatMode(chatter_key);

                    strcpy(order_type, "DISCONNECT");
                    quitting_chat = 1;
                    break; // pozniej obsluguje DISCONNECT z serverem
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

        if((message_to_server.mtype = stringToType(order_type)) == -1){
            continue;
        }
        strcpy(message_to_server.mtext, chatid_added);
        if (msgsnd(server_queueID, &message_to_server, MAX_TYPE_LENGTH, 0) == -1) {
            perror("Error while sending");
        }

        if (strcmp(order_type, "STOP") == 0) {
            printf("Finished :)\n");
            end = 1;
        }
        else if(strcmp(order_type, "CONNECT") == 0) {
            //msgrcv(queueID, &chat_invitation, MAX_MSG_LENGTH, CHAT_TYPE, MSG_NOERROR | IPC_NOWAIT);
            connecting = 1;
        }
        else {
            // oczekiwanie na komunikat od serwera o wykonaniu zlecenia
            msgrcv(queueID, &confirmation, MAX_MSG_LENGTH, CHAT_TYPE, MSG_EXCEPT);
            printf("%s\n", confirmation.mtext);
        }
    }

    return 0;
}
