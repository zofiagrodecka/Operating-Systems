#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include "utils.h"

key_t get_system_key(void){
    key_t key;
    if((key = ftok("/home", 1)) == -1){
        perror("Error while generating a key for system queue");
        exit(2);
    }
    return key;
}

key_t generate_customer_key(void){
    key_t key;
    if((key = ftok("/home", getpid())) == -1){
        perror("Error while generating a key for customer queue");
        exit(2);
    }

    return key;
}

int stringToType(char *str){
    char *pstr;
    if(strcmp(str, "INIT") == 0){
        return 4;
    }
    else if(strcmp(str, "LIST") == 0){
        return 3;
    }
    else if(strcmp(str, "DISCONNECT") == 0){
        return 2;
    }
    else if(strcmp(str, "STOP") == 0){
        return 1;
    }
    else{
        pstr = strtok(str, " ");
        if(strcmp(pstr, "CONNECT") == 0){
            return 5;
        }
    }
    perror("Wrong order type");
    return -1;
}

int isNumber(char *string){

    for(int i=0; i<strlen(string); i++){
        if(!isdigit(string[i])){
            return 0;
        }
    }
    return 1;
}
