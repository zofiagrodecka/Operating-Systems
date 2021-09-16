#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_N_ARGS 5
#define MAX_N_COMMANDS 10
#define MAX_N_COMPONENTS 10
#define MAX_STR_LEN 200
#define MAX_LINE_LENGTH 500
#define AFTER_SKLADNIK_IND 8

struct command{
    char **arguments;
    int n_args;
};

struct component{
    struct command programs[MAX_N_COMMANDS];
    int n_programs;
};

struct process{
    pid_t pid;
    int command_index;
    int program_index;
};

int charToInt(char c){
    return c - '0';
}

int componentToInt(char *name){
    int num = 0;
    for(int i=AFTER_SKLADNIK_IND; i < strlen(name); i++){
        num = 10*num + charToInt(name[i]);
    }

    num--; // indeksowane od 0
    return num;
}

int main(int argc, char *argv[]) {
    if(argc < 2){
        perror("No invocation arguments");
        exit(1);
    }

    struct component components[MAX_N_COMPONENTS]; // tablica wszystkich skladnikow
    for(int i=0; i<MAX_N_COMPONENTS; i++){
        for(int j=0; j<MAX_N_COMMANDS; j++){
            components[i].programs[j].arguments = calloc(MAX_N_ARGS+1, sizeof(char*));
            for(int k=0; k<MAX_N_ARGS+1; k++){
                components[i].programs[j].arguments[k] = calloc(MAX_STR_LEN, sizeof(char));
            }
        }
    }

    FILE *file;
    file = fopen(argv[1], "r");
    if(file == NULL){
        perror("Cannot open file");
        exit(2);
    }

    char *string = calloc(MAX_LINE_LENGTH, sizeof(char));
    char *tmp;
    char com[MAX_LINE_LENGTH];

    int s=0;
    int p=0;
    int a=0;

    fgets(string, MAX_LINE_LENGTH, file);
    if(string == NULL){
        perror("Error while reading file");
        exit(3);
    }

    while(strcmp(string, "\n") != 0){
        tmp = strtok(string, "=");
        while((tmp = strtok(NULL, "=")) != NULL){
            strcpy(com, tmp);
        }

        tmp = strtok(com, " \n");
        while(tmp != NULL){
            if(strcmp(tmp, "|") == 0){
                components[s].programs[p].n_args = a;
                p++;
                a = 0;
            }
            else {
                strcpy(components[s].programs[p].arguments[a], tmp);
                a++;
            }

            tmp = strtok(NULL, " \n");
        }

        components[s].programs[p].n_args = a;
        components[s].n_programs = p+1; // bo indeksuje od 0

        s++;
        p = 0;
        a = 0;
        fgets(string, MAX_LINE_LENGTH, file);
        if(string == NULL){
            perror("Error while reading file");
            exit(3);
        }
    }

    pid_t childPID = 1;
    pid_t parentPID = getpid();
    printf("Parent: %d\n", parentPID);

    char str_tmp[MAX_LINE_LENGTH];

    int **fd;
    char **tab;
    tab = calloc(MAX_N_ARGS+1, sizeof(char*));

    int mypid;

    char output[MAX_LINE_LENGTH];

    int line_components[MAX_N_COMPONENTS];
    int n_line_comps = 0;
    int component = 0;
    int n_line_programs = 0;

    int child_ind = 0;
    struct process *children;

    while((fgets(string, MAX_LINE_LENGTH, file)) != NULL){
        strcpy(str_tmp, string);
        printf("str_tmp: %s\n", str_tmp);
        tmp = strtok(str_tmp, " |\n");
        // tmp - skladnik pomiedzy |
        component = 0;
        n_line_comps = 0;
        n_line_programs = 0;
        while(tmp != NULL){
            //printf("tmp %s\n", tmp);
            line_components[component] = componentToInt(tmp);
            n_line_programs += components[line_components[component]].n_programs;
            n_line_comps++;
            component++;
            tmp = strtok(NULL, " |\n");
        }

        children = calloc(n_line_programs, sizeof(struct process));
        child_ind = 0;
        for(int skl=0; skl<n_line_comps; skl++){ // po skladnikach w danej linii
            for(int prog=0; prog<components[line_components[skl]].n_programs; prog++){ // po programach w danym skladniku
                children[child_ind].program_index = prog;
                children[child_ind].command_index = line_components[skl];
                child_ind++;
            }
        }

        // tworze potokow tyle ile w sumie programow w linii
        fd = calloc(n_line_programs, sizeof(int*));
        for(int i=0; i<n_line_programs; i++){
            fd[i] = calloc(2, sizeof(int));
            pipe(fd[i]);
        }

        for(int i=0; i<n_line_programs; i++){

            if(childPID != 0){ // parent tworzy tyle dzieci ile programow
                childPID = fork();

                if(getpid() == parentPID){
                    children[i].pid = childPID;
                }
                else {
                    children[i].pid = getpid();
                }
            }
        }

        if(parentPID != (mypid = getpid())){

            //printf("My pid: %d\n", mypid);

            for(int myind=0; myind < n_line_programs; myind++){
                //printf("child %d\n", children[myind].pid);
                if(mypid == children[myind].pid){ // zeby wiedziec jaki indeks ma terazniejszy proces
                    if(myind==0){
                        printf("PID: %d ind: %d\n", mypid, myind);
                        close(fd[myind][0]); // przesylam do potoku 0-wego
                        dup2(fd[myind][1], STDOUT_FILENO);

                        for(int j=1; j<n_line_programs; j++){
                            close(fd[j][0]);
                            close(fd[j][1]);
                        }

                    }
                    else{
                        printf("PID: %d ind: %d\n", mypid, myind);
                        close(fd[myind-1][1]); // wyciagam z potoku myind-1
                        dup2(fd[myind-1][0], STDIN_FILENO);

                        close(fd[myind][0]); // przesylam do potoku myind
                        dup2(fd[myind][1], STDOUT_FILENO);

                        for(int j=0; j<n_line_programs; j++){
                            if(j != myind && j != myind-1) {
                                close(fd[j][0]);
                                close(fd[j][1]);
                            }
                        }
                    }

                    for(int i=0; i<components[children[myind].command_index].programs[children[myind].program_index].n_args; i++){
                        tab[i] = components[children[myind].command_index].programs[children[myind].program_index].arguments[i];
                    }
                    tab[components[children[myind].command_index].programs[children[myind].program_index].n_args] = NULL;
                    // *************************************************************************************
                    execvp(tab[0], tab);

                    if (myind == 0){
                        close(fd[myind][1]);
                    }
                    else if (myind == n_line_programs-1){
                        close(fd[myind-1][0]);
                    }
                    free(children);
                    exit(0);
                }
            }
        }
        else{

            // IN PARENT:

            for(int i=0; i<n_line_programs; i++){
                printf("PARENT: child %d, pid %d\n", i, children[i].pid);
            }

            //printf("PARENT: listen\n");
            close(fd[n_line_programs-1][1]);

            for(int j=0; j<n_line_programs-1; j++){
                close(fd[j][0]);
                close(fd[j][1]);
            }

            while(wait(NULL) > 0);

            printf("PARENT: result\n");
            int c;
            while ((c = read(fd[n_line_programs-1][0], output, MAX_LINE_LENGTH-1)) != 0){
                output[c] = 0;
                printf("%s\n", output);
            }
            close(fd[n_line_programs-1][0]);

        }

        free(children);

    }

    fclose(file);

    free(string);
    for(int i=0; i<MAX_N_ARGS+1; i++){
        free(tab[i]);
    }
    free(tab);
    for(int i=0; i<MAX_N_COMPONENTS; i++){
        for(int j=0; j<MAX_N_COMMANDS; j++){
            for(int k=0; k<MAX_N_ARGS+1; k++){
                free(components[i].programs[j].arguments[k]);
            }
            free(components[i].programs[j].arguments);
        }
    }

    return 0;
}
