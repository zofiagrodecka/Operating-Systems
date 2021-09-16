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
#include <sys/wait.h>
#include <time.h>
#include <pthread.h>
#include <sys/time.h>
#include <errno.h>

#define _BSD_SOURCE
#define UNIX_PATH_MAX 108
#define MAX_CUSTOMERS 10
#define MAX_CUSTOMER_NAME 100
#define NET_ADDRESS "127.0.0.1"
#define NAME_EXISTS_MSG "This name is already in use"
#define WAIT_MSG "You have to wait for couple to play"
#define BEGINNER_MSG "You start with X"
#define NOTBEGINNER_MSG "You have O. Wait for your opponent's turn"
#define GAME_MSG "The game is being prepared"
#define BOARD_LEN 3
#define MAX_MESSAGE_LEN 300
#define BOARD_STR_LEN 200
#define GAME_OVER "Game over!"
#define PING "Are you there?"
#define MONITOR_PORT 1234

struct customer{
    char name[MAX_CUSTOMER_NAME];
    int fd;
    struct customer *couple;
    char game_sign;
    struct sockaddr addr;
    struct sockaddr_in net_addr;
    socklen_t custaddrlen;
    int alive;
    pid_t coordinator_pid;
    int in_game;
};

int local_sockfd = -1;
int net_sockfd = -1;
int monitor_socket = -1;
char sock_path[UNIX_PATH_MAX];
struct customer customers[MAX_CUSTOMERS];
int n = 0; // liczba klientow

void sigint_handler(int signum){
    for(int i=0; i<n; i++){
        close(customers[i].fd);
    }
    shutdown(monitor_socket, SHUT_RDWR);
    close(monitor_socket);
    shutdown(net_sockfd, SHUT_RDWR);
    shutdown(local_sockfd, SHUT_RDWR);
    unlink(sock_path);
    close(local_sockfd);
    close(net_sockfd);
    exit(0);
}

void sigterm_handler(int signum){
    exit(0);
}

void sigpipe_handler(int signum){
    printf("SIGPIPE\n");
}

void *monitor(void *args){
    char message[MAX_MESSAGE_LEN];
    struct sockaddr_in customer_monitor_addr;
    socklen_t monitorlen =  sizeof(struct sockaddr);
    fd_set fds;
    struct timeval tm;
    tm.tv_sec = 5;
    tm.tv_usec = 0;
    int counter = 0;
    int select_timeout = 0;

    while(1){
        for(int i=0; i<n; i++){
            customers[i].alive = 0;
        }

        for(int i=0; i<n; i++){
            if(customers[i].fd != -1){
                if((write(customers[i].fd, PING, sizeof(PING) + 1)) == -1){
                    customers[i].fd = -1;
                    close(customers[i].fd);
                    perror("Error while writing ping");
                }
                printf("Ping to: %s\n", customers[i].name);
            }
        }

        counter = 0;
        select_timeout = 0;

        while(counter < n && !select_timeout){

            FD_ZERO(&fds);
            FD_SET(monitor_socket, &fds);


            strcpy(message, "");

            tm.tv_sec = 5;
            tm.tv_usec = 0;

            if((select(monitor_socket+1, &fds, NULL, NULL, &tm)) == -1){
                perror("Error with select");
            }

            if(FD_ISSET(monitor_socket, &fds)){
                select_timeout = 0;
                if((recvfrom(monitor_socket, message, MAX_MESSAGE_LEN, 0, (struct sockaddr *) &customer_monitor_addr, &monitorlen)) == -1){
                    perror("Error while recvfrom");
                }
                message[strlen(message)] = '\0';
                printf("Monitor Received: %s\n", message);
                counter++;
            }
            else{
                select_timeout = 1;
            }

            if(strlen(message) != 0){
                for(int j=0; j<n; j++){
                    if(strcmp(message, customers[j].name) == 0){
                        customers[j].alive = 1;
                        break;
                    }
                }
            }

        }

        for(int i=0; i<n; i++){
            if(customers[i].fd != -1) {
                if (!customers[i].alive) {
                    customers[i].fd = -1;
                    printf("Customer: %s is dead\n", customers[i].name);

                    if ((write(customers[i].couple->fd, GAME_OVER, MAX_MESSAGE_LEN)) == -1) {
                        perror("Error while sending message to customer");
                    }

                    customers[i].couple->fd = -1;

                    if (customers[i].in_game && (kill(customers[i].coordinator_pid, SIGTERM)) == -1) {
                        perror("kill error");
                    }
                } else {
                    printf("Customer: %s is alive\n", customers[i].name);
                }
            }
        }

        sleep(5);
    }
}

int exists(char *name){
    for(int i=0; i<n; i++){
        if(customers[i].fd != -1 && strcmp(name, customers[i].name) == 0){
            return 1;
        }
    }
    return 0;
}

char *tab_to_board(char tab[BOARD_LEN][BOARD_LEN]){
    char *board = malloc(MAX_MESSAGE_LEN*sizeof(char));
    char tmp[2];
    for(int i=0; i<BOARD_LEN; i++){
        strcat(board, "| ");
        for(int j=0; j<BOARD_LEN; j++){
            tmp[0] = tab[i][j];
            tmp[1] = '\0';
            strcat(board, tmp);
            strcat(board, " | ");
        }
        strcat(board, "\n");
    }

    return board;
}

void place(char number[2], char sign, char board[BOARD_LEN][BOARD_LEN]){
    int num = number[0] - '0';
    switch(num){
        case 1:
            board[0][0] = sign;
            break;
        case 2:
            board[0][1] = sign;
            break;
        case 3:
            board[0][2] = sign;
            break;
        case 4:
            board[1][0] = sign;
            break;
        case 5:
            board[1][1] = sign;
            break;
        case 6:
            board[1][2] = sign;
            break;
        case 7:
            board[2][0] = sign;
            break;
        case 8:
            board[2][1] = sign;
            break;
        case 9:
            board[2][2] = sign;
            break;
        default:
            printf("Wrong number\n");
            break;
    }
}

char three_in_a_row(char board[BOARD_LEN][BOARD_LEN]){
    int counter = 1;
    char winner = 0;
    for(int i=0; i<BOARD_LEN; i++) {
        for (int j = 0; j < BOARD_LEN-1; j++) {
            if(board[i][j] != board[i][j+1]){
                break;
            }
            else{
                winner = board[i][j];
                counter++;
            }
        }
        if(counter == 3){
            return winner;
        }
        counter = 1;
    }

    counter = 1;
    for(int i=0; i<BOARD_LEN; i++) {
        for (int j = 0; j < BOARD_LEN-1; j++) {
            if(board[j][i] != board[j+1][i]){
                break;
            }
            else{
                winner = board[j][i];
                counter++;
            }
        }
        if(counter == 3){
            return winner;
        }
        counter = 1;
    }

    counter = 1;
    for(int i=0; i<BOARD_LEN-1; i++) {
        if(board[i][i] != board[i+1][i+1]){
            break;
        }
        else{
            winner = board[i][i];
            counter++;
        }
    }
    if(counter == 3){
        return winner;
    }

    if(board[0][2] == board[1][1] && board[1][1] == board[2][0]){
        winner = board[0][2];
        return winner;
    }

    return 'C' ; // continue
}

int main(int argc, char *argv[]) {

    if(argc != 3){
        perror("Wrong number of invocation arguments");
        return 1;
    }

    printf("Hello, World!\n");

    struct timeval tv;
    gettimeofday(&tv, NULL);
    srand(tv.tv_sec+tv.tv_usec);

    signal(SIGINT, sigint_handler);
    signal(SIGTERM, sigterm_handler);
    signal(SIGPIPE, sigpipe_handler);

    in_port_t port = strtol(argv[1], NULL, 10);
    strcpy(sock_path, argv[2]);

    if((local_sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) { // socket()
        perror("Cannot create local socket");
    }

    int optval = 1;
    if((setsockopt(local_sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int))) == -1){
        perror("Error with setsockopt");
    }

    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, sock_path);
    if(bind(local_sockfd, (struct sockaddr *) &addr, sizeof(struct sockaddr)) == -1) { // bind()
        unlink(sock_path);
        perror("Cannot bind local socket");
    }

    if((listen(local_sockfd, MAX_CUSTOMERS)) == -1){ // listen()
        perror("Cannot listen lockal socket");
    }

    if((net_sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) { // socket()
        perror("Cannot create network socket");
    }

    int optval2 = 1;
    if((setsockopt(net_sockfd, SOL_SOCKET, SO_REUSEADDR, &optval2, sizeof(int))) == -1){
        perror("Error with setsockopt");
    }

    struct sockaddr_in net_addr;
    net_addr.sin_family = AF_INET;
    net_addr.sin_port = port;

    struct in_addr inaddr;
    if((inet_pton(AF_INET, NET_ADDRESS, &inaddr)) == -1){
        perror("Error converting string with an address to struct in_addr");
    }
    net_addr.sin_addr = inaddr;

    if(bind(net_sockfd, (struct sockaddr *) &net_addr, sizeof(struct sockaddr)) == -1) { // bind()
        perror("Cannot bind network socket");
    }

    if((listen(net_sockfd, MAX_CUSTOMERS)) == -1){ // listen()
        perror("Cannot listen network socket");
    }

    // monitor socket
    if((monitor_socket = socket(AF_INET, SOCK_DGRAM, 0)) == -1) { // socket()
        perror("Cannot create monitor socket");
    }

    struct sockaddr_in monitor_addr;
    monitor_addr.sin_family = AF_INET;
    monitor_addr.sin_port = htons(MONITOR_PORT);

    struct in_addr in_monitor_addr;
    if((inet_pton(AF_INET, NET_ADDRESS, &in_monitor_addr)) == -1){
        perror("Error converting string with an address to struct in_addr");
    }
    monitor_addr.sin_addr = in_monitor_addr;

    if(bind(monitor_socket, (struct sockaddr *) &monitor_addr, sizeof(struct sockaddr)) == -1) { // bind()
        perror("Cannot bind monitor socket");
    }

    // koniec monitora

    for(int i=0; i<MAX_CUSTOMERS; i++){
        customers[i].custaddrlen = sizeof(struct sockaddr);
        customers[i].fd = -1;
    }

    fd_set sockets;

    int max = 0;
    if(local_sockfd > net_sockfd){
        max = local_sockfd;
    }
    else{
        max = net_sockfd;
    }

    char customer_name[MAX_CUSTOMER_NAME];
    struct customer *free_customer = NULL;
    int beginnerfd, not_beginnerfd;
    char board[BOARD_LEN][BOARD_LEN];
    char tmp[MAX_MESSAGE_LEN];
    char move[5];
    char winning = 0;
    int winner, loser;
    char num = '1';
    int free_moves = 0;
    int draw = 0;
    pid_t childpid;

    pthread_t tid;
    if((pthread_create(&tid, NULL, monitor, NULL)) != 0){
        perror("Cannot create thread");
    }

    while(1){

        FD_ZERO(&sockets);
        FD_SET(local_sockfd, &sockets);
        FD_SET(net_sockfd, &sockets);

        if((select(max+1, &sockets, NULL, NULL, NULL)) == -1){
            perror("Error with select");
        }

        if(FD_ISSET(local_sockfd, &sockets)){
            if((customers[n].fd = accept(local_sockfd, &customers[n].addr, &customers[n].custaddrlen)) == -1){ //accept()
                perror("Cannot accept connections to local socket");
            }
        }
        else if(FD_ISSET(net_sockfd, &sockets)){
            if((customers[n].fd = accept(net_sockfd, &customers[n].addr, &customers[n].custaddrlen)) == -1){ //accept()
                perror("Cannot accept connections to local socket");
            }
        }

        if((read(customers[n].fd, customer_name, MAX_CUSTOMER_NAME)) == -1){
            perror("Error while reading from local socket");
        }
        customer_name[strlen(customer_name)] = '\0';
        printf("Read message: %s\n", customer_name);

        if(exists(customer_name)){
            if((write(customers[n].fd, NAME_EXISTS_MSG, sizeof(NAME_EXISTS_MSG)+1)) == -1){
                perror("Error while answering name to customer");
            }
        }
        else{
            strcpy(customers[n].name, customer_name); // zapamietuje klienta
            if(n % 2 == 0){ // dodaje klienta bez pary
                customers[n].couple = NULL;
                free_customer = &customers[n];

                if((write(customers[n].fd, WAIT_MSG, sizeof(WAIT_MSG)+1)) == -1){
                    perror("Error while answering wait to customer");
                }
            }
            else{
                customers[n].couple = free_customer;
                free_customer->couple = &customers[n];

                if(rand() % 2 == 0){
                    customers[n].in_game = 0;
                    beginnerfd = customers[n].fd;
                    not_beginnerfd = free_customer->fd;
                    customers[n].game_sign = 'X';
                    free_customer->game_sign = 'O';
                }
                else {
                    customers[n].in_game = 1;
                    free_customer->in_game = 1;
                    not_beginnerfd = free_customer->fd;
                    beginnerfd = customers[n].fd;
                    free_customer->game_sign = 'O';
                    customers[n].game_sign = 'X';
                }

                num = '1';
                for(int i=0; i<BOARD_LEN; i++){
                    for(int j=0; j<BOARD_LEN; j++){
                        board[i][j] = num;
                        num++;
                    }
                }

                if((write(beginnerfd, GAME_MSG, MAX_MESSAGE_LEN)) == -1){
                    perror("Error while answering to customer");
                }

                strcpy(tmp, tab_to_board(board));
                strcat(tmp, BEGINNER_MSG);
                if((write(beginnerfd, tmp, MAX_MESSAGE_LEN)) == -1){
                    perror("Error while answering to customer");
                }

                if((write(not_beginnerfd, GAME_MSG, MAX_MESSAGE_LEN)) == -1){
                    perror("Error while answering to customer");
                }

                if((write(not_beginnerfd, NOTBEGINNER_MSG, MAX_MESSAGE_LEN)) == -1){
                    perror("Error while answering to customer");
                }

                free_moves = 9;

               if((childpid = fork()) == 0){ // Game
                    close(net_sockfd);
                    close(local_sockfd);
                    while(free_moves > 0){
                        if((read(beginnerfd, move, MAX_MESSAGE_LEN)) == -1){
                            perror("Error while reading customer move");
                        }
                        printf("Read move: %s\n", move);
                        place(move, 'X', board);
                        free_moves--;
                        if(((winning = three_in_a_row(board)) != 'C') || (free_moves == 0)){
                            break;
                        }

                        strcpy(tmp, tab_to_board(board));
                        if((write(not_beginnerfd, tmp, MAX_MESSAGE_LEN)) == -1){
                            perror("Error while sending opponent's move to customer");
                        }

                        printf("Board:\n%s\n", tmp);

                        if((read(not_beginnerfd, move, MAX_MESSAGE_LEN)) == -1){
                            perror("Error while reading customer move");
                        }
                        place(move, 'O', board);
                        free_moves--;

                        if(((winning = three_in_a_row(board)) != 'C') || (free_moves == 0)){
                            break;
                        }

                        strcpy(tmp, tab_to_board(board));
                        if((write(beginnerfd, tmp, MAX_MESSAGE_LEN)) == -1){
                            perror("Error while sending opponent's move to beginner customer");
                        }
                    }

                    printf("END GAME, winning: %c\n", winning);

                   if((write(beginnerfd, GAME_OVER, MAX_MESSAGE_LEN)) == -1){
                        perror("Error while sending message to customer");
                    }
                    if((write(not_beginnerfd, GAME_OVER, MAX_MESSAGE_LEN)) == -1){
                        perror("Error while sending message to customer");
                    }

                    if(customers[n].game_sign == winning){
                        winner = customers[n].fd;
                        loser = free_customer->fd;
                    }
                    else if(free_customer->game_sign == winning){
                        winner = free_customer->fd;
                        loser = customers[n].fd;
                    }
                    else{
                        draw = 1;
                    }

                    if(draw){
                        if((write(beginnerfd, "No one won :)", MAX_MESSAGE_LEN)) == -1){
                            perror("Error while sending message to customer");
                        }
                        if((write(not_beginnerfd, "No one won :)", MAX_MESSAGE_LEN)) == -1){
                            perror("Error while sending message to customer");
                        }
                    }
                    else{
                        if((write(winner, "YOU WON! CONGRATULATIONS!", MAX_MESSAGE_LEN)) == -1){
                            perror("Error while sending message to customer");
                        }
                        if((write(loser, "YOU LOST :(", MAX_MESSAGE_LEN)) == -1){
                            perror("Error while sending message to customer");
                        }
                    }

                    exit(0);
                }
               else{
                   customers[n].coordinator_pid = childpid;
                   free_customer->coordinator_pid = childpid;
               }
            }

            n++;
        }
    }

    return 0;
}
