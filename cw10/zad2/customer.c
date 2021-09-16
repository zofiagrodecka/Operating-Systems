#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

#define _BSD_SOURCE
#define UNIX_PATH_MAX 108
#define MAX_CUSTOMERS 10
#define MAX_CUSTOMER_NAME 100
#define MAX_ADDRESS 30
#define MAX_MESSAGE_LEN 300
#define NAME_EXISTS_MSG "This name is already in use"
#define WAIT_MSG "You have to wait for couple to play"
#define GAME_MSG "The game is being prepared"
#define BOARD_LEN 3
#define GAME_OVER "Game over!"
#define BEGINNER_MSG "You start with X"
#define NOTBEGINNER_MSG "You have O. Wait for your opponent's turn"
#define PING "Are you there?"
#define CONFIRMATION "I am alive"
#define MONITOR_PORT 1234
#define NET_ADDRESS "127.0.0.1"

enum connection_type{
    NETWORK,
    LOCAL
};

int sockfd = -1;
int monitor_socket = -1;

void sigint_handler(int signum){
    shutdown(monitor_socket, SHUT_RDWR);
    close(monitor_socket);
    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);
    exit(0);
}

int main(int argc, char *argv[]) {

    if(argc != 4 && argc != 5){
        perror("Wrong number of invocation arguments");
        return 1;
    }

    printf("Hello, World!\n");

    signal(SIGINT, sigint_handler);

    char myname[MAX_CUSTOMER_NAME];
    strcpy(myname, argv[1]);

    enum connection_type connection;
    if(strcmp(argv[2], "network") == 0){
        connection = NETWORK;
    }
    else if(strcmp(argv[2], "local") == 0){
        connection = LOCAL;
    }
    else{
        perror("Wrong type of connection");
        return 1;
    }

    char socket_path[UNIX_PATH_MAX];
    in_port_t port;

    struct in_addr inaddr;

    if(argc == 4){ // tylko sciezka do gniazda
        strcpy(socket_path, argv[3]);
    }
    else{ // adres IPv4 i numer portu
        port = strtol(argv[4], NULL, 10);
    }

    if((inet_pton(AF_INET, NET_ADDRESS, &inaddr)) == -1){
        perror("Error converting string with an address to struct in_addr");
    }

    struct sockaddr_in addr;
    struct sockaddr *sockaddr;
    struct sockaddr_un unaddr;

    if(connection == LOCAL){
        if((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1) { // socket()
            perror("Cannot create local socket");
        }

        int optval = 1;
        if((setsockopt(sockfd, SOL_SOCKET, SO_PASSCRED, &optval, sizeof(int))) == -1){
            perror("Error with setsockopt");
        }

        unaddr.sun_family = AF_UNIX;
        strcpy(unaddr.sun_path, socket_path);
        sockaddr = (struct sockaddr *) &unaddr;
    }
    else{
        if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) { // socket()
            perror("Cannot create network socket");
        }

        addr.sin_family = AF_INET;
        addr.sin_port = port;
        addr.sin_addr = inaddr;
        sockaddr = (struct sockaddr *) &addr;
    }

    //monitor
    if((monitor_socket = socket(AF_INET, SOCK_DGRAM, 0)) == -1) { // socket()
        perror("Cannot create monitor socket");
    }

    struct sockaddr_in monitor_server_addr;
    monitor_server_addr.sin_family = AF_INET;
    monitor_server_addr.sin_port = htons(MONITOR_PORT);
    monitor_server_addr.sin_addr = inaddr;
    socklen_t monitoraddrlen =  sizeof(struct sockaddr_in);
    //end monitor

    socklen_t serveraddrlen =  sizeof(struct sockaddr);

    if((sendto(sockfd, myname, sizeof(myname), 0, sockaddr, serveraddrlen)) == -1){
        perror("Error while sendto");
    }
    printf("Sent my name to server\n");

    char message[MAX_MESSAGE_LEN];
    char move[5];
    int beginner = 0;

    while(1){
        if((recvfrom(sockfd, message, MAX_MESSAGE_LEN, MSG_WAITALL, sockaddr, &serveraddrlen)) == -1){
            perror("Error while recvfrom");
        }
        printf("Read message: %s\n", message);

        if(strcmp(message, PING) == 0){
            if((sendto(monitor_socket, myname, sizeof(myname), 0, (struct sockaddr *) &monitor_server_addr, serveraddrlen)) == -1){
                perror("Error while sending reply to ping");
            }
            printf("Sent response ping: %s\n", myname);
        }
        else if(strcmp(message, GAME_MSG) == 0) { // GAME

            if((recvfrom(sockfd, message, MAX_MESSAGE_LEN, MSG_WAITALL, sockaddr, &serveraddrlen)) == -1){
                perror("Error while recvfrom");
            }
            message[strlen(message)] = '\0';
            printf("%s\n", message);

            if (strcmp(message, NOTBEGINNER_MSG) == 0) {
                beginner = 0;
            } else {
                beginner = 1;
            }
            printf("BEGINNER: %d\n", beginner);

            if(beginner){
                while(1){
                    if(fork() == 0){
                        printf("Type your move:\n");
                        scanf(" %s", move);

                        if((sendto(sockfd, move, sizeof(move), 0, sockaddr, serveraddrlen)) == -1){
                            perror("Error while sendto");
                        }
                        printf("Sent: %s\n", move);
                        close(sockfd);
                        exit(0);
                    }

                    if((recvfrom(sockfd, message, MAX_MESSAGE_LEN, MSG_WAITALL, sockaddr, &serveraddrlen)) == -1){
                        perror("Error while recvfrom");
                    }
                    message[strlen(message)] = '\0';
                    printf("%s\n", message);

                    while(strcmp(message, PING) == 0){
                        if((sendto(monitor_socket, myname, sizeof(myname), 0, (struct sockaddr *) &monitor_server_addr, serveraddrlen)) == -1){
                            perror("Error while sending reply to ping");
                        }
                        printf("Sent response ping: %s\n", myname);
                        if((read(sockfd, message, MAX_MESSAGE_LEN)) == -1){
                            perror("Error while reading from socket");
                        }
                        message[strlen(message)] = '\0';
                        printf("%s\n", message);
                    }

                    if(strcmp(message, GAME_OVER) == 0){
                        break;
                    }
                }
            }
            else {
                while (1) {
                    if ((recvfrom(sockfd, message, MAX_MESSAGE_LEN, MSG_WAITALL, sockaddr, &serveraddrlen)) == -1) {
                        perror("Error while recvfrom");
                    }
                    message[strlen(message)] = '\0';
                    printf("%s\n", message);

                    while(strcmp(message, PING) == 0){
                        if((sendto(monitor_socket, myname, sizeof(myname), 0, (struct sockaddr *) &monitor_server_addr, serveraddrlen)) == -1){
                            perror("Error while sending reply to ping");
                        }
                        printf("Sent response ping: %s\n", myname);
                        if((read(sockfd, message, MAX_MESSAGE_LEN)) == -1){
                            perror("Error while reading from socket");
                        }
                        message[strlen(message)] = '\0';
                        printf("%s\n", message);
                    }

                    if (strcmp(message, GAME_OVER) == 0) {
                        break;
                    }

                    if (fork() == 0) {
                        printf("Type your move:\n");
                        scanf(" %s", move);

                        if ((sendto(sockfd, move, sizeof(move), 0, sockaddr, serveraddrlen)) == -1) {
                            perror("Error while sendto");
                        }
                        printf("Sent: %s\n", move);
                        close(sockfd);
                        exit(0);
                    }
                }
            }
        }
    }

    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);

    return 0;

}
