#include <stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include <unistd.h>
#include<time.h>
#include"functions.h"

int strToInt(char * str){
    return strtol(str, NULL, 10);
}

int even(int x){
    if(x % 2 == 0){
        return 1;
    }
    return 0;
}

int check_tens_digit(int x){
    int r = x % 10;
    if(x > 9) {
        x /= 10;
        r = x % 10;
        if (r == 0 || r == 7) {
            return 1;
        }
    }
    return 0;
}

int is_square(int x){
    for(int i=1; i*i <= x; i++){
        if(i*i == x){
            return 1;
        }
    }
    return 0;
}
