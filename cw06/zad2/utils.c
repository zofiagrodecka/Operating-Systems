#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <limits.h>
#include "utils.h"

char *get_customer_queue_name(void){
    char *str = calloc(MAX_QNAME_LENGTH, sizeof(char));
    strcpy(str, "/");
    char tmp[MAX_QNAME_LENGTH];

    sprintf(tmp, "%d", getpid());

    strcat(str, tmp);

    return str;
}

char *get_server_queue_name(void){
    char *str = calloc(10, sizeof(char));
    strcpy(str, "/server");
    return str;
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
