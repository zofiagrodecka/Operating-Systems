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
#include<sys/time.h>
#include <sys/wait.h>
#include <pthread.h>

#define _BSD_SOURCE
#define UNIX_PATH_MAX 108
#define MAX_CUSTOMERS 10
#define MAX_CUSTOMER_NAME 100
#define NET_ADDRESS "127.0.0.1"
#define GAME_LOCAL_SOCK "Game_socket"
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
    int connected_by_net;
};

int local_sockfd = -1;
int net_sockfd = -1;
int monitor_socket = -1;
int game_socket = -1;
int game_local_socket = -1;
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
    shutdown(game_local_socket, SHUT_RDWR);
    shutdown(game_socket, SHUT_RDWR);
    close(game_socket);
    close(game_local_socket);
    unlink(GAME_LOCAL_SOCK);
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
                if(customers[i].in_game){
                    if((sendto(customers[i].fd, PING, sizeof(PING), 0, &customers[i].addr, customers[i].custaddrlen)) == -1){
                        customers[i].fd = -1;
                        close(customers[i].fd);
                        perror("Error while sending ping to in game customer");
                    }
                }
                else{
                    if((sendto(customers[i].fd, PING, sizeof(PING), 0, &customers[i].addr, customers[i].custaddrlen)) == -1){
                        customers[i].fd = -1;
                        close(customers[i].fd);
                        perror("Error while sending ping to customer");
                    }
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
                    if(customers[i].couple != NULL){
                        if((sendto(customers[i].couple->fd, GAME_OVER, sizeof(GAME_OVER), 0, &customers[i].couple->addr, customers[i].couple->custaddrlen)) == -1){
                            customers[i].fd = -1;
                            perror("Error while sending game over to customer");
                        }

                        customers[i].couple->fd = -1;
                    }

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
    printf("%d\n", num);
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

    if((local_sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1) { // socket() protocol=0?
        perror("Cannot create local socket");
    }

    struct sockaddr_un loc_addr;
    loc_addr.sun_family = AF_UNIX;
    strcpy(loc_addr.sun_path, sock_path);
    if(bind(local_sockfd, (struct sockaddr *) &loc_addr, sizeof(struct sockaddr)) == -1) { // bind()
        unlink(sock_path);
        perror("Cannot bind local socket");
    }

    if((net_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) { // socket()
        perror("Cannot create network socket");
    }

    int optval = 1;
    if((setsockopt(net_sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int))) == -1){
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

    if((game_socket = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
        perror("Cannot create game socket");
    }
    struct sockaddr_in game_addr;
    game_addr.sin_family = AF_INET;
    game_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    game_addr.sin_port = htons(0);
    if((bind(game_socket, (const struct sockaddr *) &game_addr, sizeof(game_addr))) == -1){
        perror("Error while binding game socket");
    }

    if((game_local_socket = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1){
        perror("Cannot create game local socket");
    }
    struct sockaddr_un game_unaddr;
    game_unaddr.sun_family = AF_UNIX;
    strcpy(game_unaddr.sun_path, GAME_LOCAL_SOCK);
    if((bind(game_local_socket, (const struct sockaddr *) &game_unaddr, sizeof(game_unaddr))) == -1){
        perror("Error while binding game local socket");
    }

    for(int i=0; i<MAX_CUSTOMERS; i++){
        customers[i].custaddrlen = sizeof(struct sockaddr);
        customers[i].fd = -1;
        customers[i].couple = NULL;
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
    int beginner_index, not_beginner_index;
    char board[BOARD_LEN][BOARD_LEN];
    char tmp[MAX_MESSAGE_LEN];
    char move[5];
    char winning = 0;
    int winner, loser;
    char num = '1';
    int free_moves = 0;
    int draw = 0;
    pid_t childpid;
    int x = 0;

    pthread_t tid;
    if((pthread_create(&tid, NULL, monitor, NULL)) != 0){
        perror("Cannot create thread");
    }

    while(1) {

        FD_ZERO(&sockets);
        FD_SET(local_sockfd, &sockets);
        FD_SET(net_sockfd, &sockets);

        if ((select(max + 1, &sockets, NULL, NULL, NULL)) == -1) {
            perror("Error with select");
        }

        if (FD_ISSET(local_sockfd, &sockets)) {
            if ((recvfrom(local_sockfd, customer_name, MAX_CUSTOMER_NAME, MSG_WAITALL, &customers[n].addr, &customers[n].custaddrlen)) == -1) {
                perror("Error while reading name from local socket");
            }
            customers[n].fd = local_sockfd;
            customers[n].connected_by_net = 0;
        } else if (FD_ISSET(net_sockfd, &sockets)) {
            if ((recvfrom(net_sockfd, customer_name, MAX_CUSTOMER_NAME, MSG_WAITALL, &customers[n].addr, &customers[n].custaddrlen)) == -1) {
                perror("Error while reading name from net socket");
            }
            customers[n].fd = net_sockfd;
            customers[n].connected_by_net = 1;
        }

        customer_name[strlen(customer_name)] = '\0';
        printf("Read message: %s\n", customer_name);

        if(exists(customer_name)){
            if((sendto(customers[n].fd, NAME_EXISTS_MSG, sizeof(NAME_EXISTS_MSG), 0, &customers[n].addr, customers[n].custaddrlen)) == -1){
                perror("Error while answering to customer");
            }
        }
        else{
            strcpy(customers[n].name, customer_name); // zapamietuje klienta
            if(n % 2 == 0){ // dodaje klienta bez pary
                customers[n].in_game = 0;
                customers[n].couple = NULL;
                free_customer = &customers[n];

                if((sendto(customers[n].fd, WAIT_MSG, sizeof(WAIT_MSG), 0, &customers[n].addr, customers[n].custaddrlen)) == -1){
                    perror("Error while answering to customer");
                }
            }
            else {
                customers[n].in_game = 1;
                customers[n].couple = free_customer;
                free_customer->couple = &customers[n];
                free_customer->in_game = 1;

                if (rand() % 2 == 0) {
                    beginner_index = n;
                    x = 0;
                    while(strcmp(customers[x].name, free_customer->name) != 0){
                        x++;
                    }
                    not_beginner_index = x;

                    customers[n].game_sign = 'X';
                    free_customer->game_sign = 'O';
                } else {
                    x = 0;
                    while(strcmp(customers[x].name, free_customer->name) != 0){
                        x++;
                    }
                    not_beginner_index = x;
                    beginner_index = n;
                    free_customer->game_sign = 'O';
                    customers[n].game_sign = 'X';
                }

                if(customers[beginner_index].connected_by_net){
                    customers[beginner_index].fd = game_socket;
                }
                else{
                    customers[beginner_index].fd = game_local_socket;
                }

                if(customers[not_beginner_index].connected_by_net){
                    customers[not_beginner_index].fd = game_socket;
                }
                else{
                    customers[not_beginner_index].fd = game_local_socket;
                }

                num = '1';
                for (int i = 0; i < BOARD_LEN; i++) {
                    for (int j = 0; j < BOARD_LEN; j++) {
                        board[i][j] = num;
                        num++;
                    }
                }

                if((childpid = fork()) == 0){
                    close(net_sockfd);
                    close(local_sockfd);

                    customers[beginner_index].custaddrlen = sizeof(game_addr);
                    if((sendto(customers[beginner_index].fd, GAME_MSG, sizeof(GAME_MSG), 0, &customers[beginner_index].addr, customers[beginner_index].custaddrlen)) == -1){
                        perror("Error while answering in game to customer");
                    }

                    strcpy(tmp, tab_to_board(board));
                    strcat(tmp, BEGINNER_MSG);
                    if((sendto(customers[beginner_index].fd, tmp, sizeof(tmp), 0, &customers[beginner_index].addr, customers[beginner_index].custaddrlen)) == -1){
                        perror("Error while answering to customer");
                    }

                    if((sendto(customers[not_beginner_index].fd, GAME_MSG, sizeof(GAME_MSG), 0, &customers[not_beginner_index].addr, customers[not_beginner_index].custaddrlen)) == -1){
                        perror("Error while answering to customer");
                    }

                    if((sendto(customers[not_beginner_index].fd, NOTBEGINNER_MSG, sizeof(NOTBEGINNER_MSG), 0, &customers[not_beginner_index].addr, customers[not_beginner_index].custaddrlen)) == -1){
                        perror("Error while answering to customer");
                    }

                    free_moves = 9;

                    while(free_moves > 0){
                        if((recvfrom(customers[beginner_index].fd, move, sizeof(move), MSG_WAITALL, &customers[beginner_index].addr, &customers[beginner_index].custaddrlen)) == -1) {
                            perror("Error while reading from socket");
                        }
                        printf("Read move: %s\n", move);
                        place(move, 'X', board);
                        free_moves--;
                        if(((winning = three_in_a_row(board)) != 'C') || (free_moves == 0)){
                            break;
                        }

                        strcpy(tmp, tab_to_board(board));
                        if((sendto(customers[not_beginner_index].fd, tmp, sizeof(tmp), 0, &customers[not_beginner_index].addr, customers[not_beginner_index].custaddrlen)) == -1){
                            perror("Error while answering to customer");
                        }

                        printf("Board:\n%s\n", tmp);

                        if((recvfrom(customers[not_beginner_index].fd, move, sizeof(move), MSG_WAITALL, &customers[not_beginner_index].addr, &customers[not_beginner_index].custaddrlen)) == -1) {
                            perror("Error while reading from socket");
                        }
                        place(move, 'O', board);
                        free_moves--;

                        if(((winning = three_in_a_row(board)) != 'C') || (free_moves == 0)){
                            break;
                        }

                        strcpy(tmp, tab_to_board(board));
                        if((sendto(customers[beginner_index].fd, tmp, sizeof(tmp), 0, &customers[beginner_index].addr, customers[beginner_index].custaddrlen)) == -1){
                            perror("Error while answering to customer");
                        }
                    }

                    printf("END GAME, winning: %c\n", winning);

                    if((sendto(customers[beginner_index].fd, GAME_OVER, sizeof(GAME_OVER), 0, &customers[beginner_index].addr, customers[beginner_index].custaddrlen)) == -1){
                        perror("Error while answering to customer");
                    }
                    if((sendto(customers[not_beginner_index].fd, GAME_OVER, sizeof(GAME_OVER), 0, &customers[not_beginner_index].addr, customers[not_beginner_index].custaddrlen)) == -1){
                        perror("Error while answering to customer");
                    }

                    if(customers[beginner_index].game_sign == winning){
                        winner = beginner_index;
                        loser = not_beginner_index;
                    }
                    else if(customers[not_beginner_index].game_sign == winning){
                        winner = not_beginner_index;
                        loser = beginner_index;
                    }
                    else{
                        draw = 1;
                    }

                    if(draw){
                        if((sendto(customers[beginner_index].fd, "No one won :)", MAX_MESSAGE_LEN, 0, &customers[beginner_index].addr, customers[beginner_index].custaddrlen)) == -1){
                            perror("Error while answering to customer");
                        }
                        if((sendto(customers[not_beginner_index].fd, "No one won :)", MAX_MESSAGE_LEN, 0, &customers[not_beginner_index].addr, customers[not_beginner_index].custaddrlen)) == -1){
                            perror("Error while answering to customer");
                        }
                    }
                    else{
                        if((sendto(customers[winner].fd, "YOU WON! CONGRATULATIONS!", MAX_MESSAGE_LEN, 0, &customers[winner].addr, customers[winner].custaddrlen)) == -1){
                            perror("Error while answering to customer");
                        }
                        if((sendto(customers[loser].fd, "YOU LOST :(", MAX_MESSAGE_LEN, 0, &customers[loser].addr, customers[loser].custaddrlen)) == -1){
                            perror("Error while answering to customer");
                        }
                    }

                    customers[n].in_game = 0;
                    free_customer->in_game = 0;

                    close(game_socket);
                    close(game_local_socket);
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

    shutdown(net_sockfd, SHUT_RDWR);
    shutdown(local_sockfd, SHUT_RDWR);
    unlink(sock_path);

    return 0;
}
