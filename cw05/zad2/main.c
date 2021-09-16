#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARGS_N1 1
#define ARGS_N2 3
#define EMAIL_IND 1
#define SUBJECT_IND 2
#define CONTENT_IND 3
#define MAX_N_CHARS 200
#define MAX_SUBJECT_LENGTH 100
#define MAX_EMAILNAME_LENGTH 50

void print_emails(FILE *mail_out){
    char str[MAX_N_CHARS];
    while(fgets(str, MAX_N_CHARS-1, mail_out) != NULL){
        puts(str);
    }
}

int main(int argc, char *argv[]) {
    FILE * mail_output;
    FILE *mail_input;

    if(argc == ARGS_N1+1){ // 1 argument: nadawca lub data
        if(strcmp(argv[ARGS_N1], "nadawca") == 0){
            mail_output = popen("mail -H | sort -k3", "r");
            print_emails(mail_output);
            pclose(mail_output);
        }
        else if(strcmp(argv[ARGS_N1], "data") == 0){
            mail_output = popen("mail -H | sort -k4", "r");
            print_emails(mail_output);
            pclose(mail_output);
        }
        else{
            perror("Wrong first argument");
            exit(2);
        }
    }
    else if(argc == ARGS_N2+1){
        char command[MAX_SUBJECT_LENGTH + MAX_EMAILNAME_LENGTH + 10] = "mail -s ";
        strcat(command, argv[SUBJECT_IND]);
        strcat(command, " ");
        strcat(command, argv[EMAIL_IND]);
        puts(command);
        mail_input = popen(command, "w");
        fputs(argv[CONTENT_IND], mail_input);

        printf("Mail sent\n");
        pclose(mail_input);
    }
    else{
        perror("Wrong number of arguments");
        exit(2);
    }

    return 0;
}
