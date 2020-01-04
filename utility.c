#include "myshell.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <dirent.h>

// initialize the struct
void initialize(command* cmd){
    cmd->arg[0] = NULL;
    cmd->argc = 0;
    cmd->background = 0;
    cmd->file = NULL;
    cmd->inputMod = 0;
    cmd->outputMod = 0;
    cmd->parallel = 0;
    cmd->pipe = 0;
    cmd->one = NULL;
}

// free everything in struct
void freeStruct(command* cmd){
    for(int i = 0; i < cmd->argc; i++){
        free(cmd->arg[i]);
    }
    if(cmd->file){
        free(cmd->file);
    }
    
    command* cmd1;
    command* one = cmd->one;
    while(one != NULL){
        cmd1 = one;
        one = one->one;
        free(cmd1);
    }
}

void printCommand(command* cmd){
    printf("\nargs: ");
    for(int i = 0; i < cmd->argc && cmd->arg[i] != NULL; i++){
        printf("%s ", cmd->arg[i]);
    }
    puts("");
    printf("argc: %d\nfilename: %s\ninput: %d\noutput: %d\nbackground: %d\nparallel: %d\npipe:%d\n",
                                                                                    cmd->argc, 
                                                                                    cmd->file, 
                                                                                    cmd->inputMod, 
                                                                                    cmd->outputMod, 
                                                                                    cmd->background, 
                                                                                    cmd->parallel, 
                                                                                    cmd->pipe);

    // linked command
    if(cmd->one == NULL){
        puts("");
        // printf("\n--------------------END\n");
    }else{// cmd->one != NULL
        printCommand(cmd->one);
    }
}

void parsePipe(command* cmd, char* line){
    // 1. new string
    char* line_dup = strdup(line);
    // 2. change cmd.parallel
    cmd->pipe = 1;

    // tokenize string twice
    char* token = NULL;
    char* saveptr;
    token = strtok_r(line_dup, "|", &saveptr);

    // check if tokens are NULL
    if(token != NULL){
        cmd->pipe = 1;
        parseSpaces(cmd, token);
    }

    if(saveptr != NULL){
        cmd->one = malloc(sizeof(command));
        initialize(cmd->one);
        parseSpaces(cmd->one, saveptr);
    }
}

// commandName arg1 arg2
void parseSpaces(command* cmd, char* line){
    // printf("the line is:%s\n", line);
    char* line_dup = strdup(line);
    char* token = NULL;
    char* saveptr;

    int index = 0;
    token = strtok_r(line_dup, " ", &saveptr);
    while(token != NULL){
        cmd->arg[index] = strdup(token);
        // printf("%d\t%s\n", index, cmd->arg[index]);
        index+=1;
        token = strtok_r(NULL, " ", &saveptr);
    }
    cmd->argc = index;
    cmd->arg[index] = NULL;
    free(line_dup);
}


void parseBackground(command* cmd, char* line){
    cmd->background = 1;
    char* ptr = line;
    int index = 0;
    while( *(ptr+index) != '\0' ){
        if(*(ptr+index) == '&'){
            *(ptr+index) = '\0';
        }
        index+=1;
    }
    parseSpaces(cmd, line);
}

void parseParallel(command* cmd, char* line){
    // create new string
    char* line_dup = strdup(line);
    char* saveptr;
    char* token;
    token = strtok_r(line_dup, "&", &saveptr);
    // printf("token: %-10s saveptr: %s\n", token, saveptr);
    
    if(token != NULL){
        cmd->parallel = 1;
        parseSpaces(cmd, token);
    }

    if(saveptr != NULL){
        cmd->one = malloc(sizeof(command));
        initialize(cmd->one);
        parseSpaces(cmd->one, saveptr);
    }
    
}
// redirection
// 2 for "<<"
// 1 for "<"
void input_redirection(command* cmd, char* line){
    
    // duplicate of line
    char* line_dup = strdup(line);
    
    // tokenize line_dup
    char* saveptr;
    char* token;
    token = strtok_r(line_dup, "<", &saveptr);
    if(token != NULL){
        // printf("token: %s\n", token);
        parseSpaces(cmd, token);
    }
    if(saveptr != NULL){
        // "<<"
        if(*saveptr == '<'){
            cmd->inputMod = 2;
            saveptr = saveptr+1;
        }
        // printf("saveptr: %s\n", saveptr);
        while(*saveptr == ' '){
            saveptr+=1;
        }
        cmd->file = strdup(saveptr);
    }
}

// redirection
// 2 for ">>"
// 1 for ">"
void output_redirection(command* cmd, char* line){
    
    // duplicate of line
    char* line_dup = strdup(line);
    
    // tokenize line_dup
    char* saveptr;
    char* token;
    token = strtok_r(line_dup, ">", &saveptr);
    if(token != NULL){
        // printf("token: %s\n", token);
        parseSpaces(cmd, token);
    }
    if(saveptr != NULL){
        // check if it is ">>"
        if(*saveptr == '>'){
            cmd->outputMod = 2;
            saveptr = saveptr+1;
        }else{
            cmd->outputMod = 1;
        }
        // printf("saveptr: %s\n", saveptr);
        while(*saveptr == ' '){
            saveptr+=1;
        }
        cmd->file = strdup(saveptr);
    }
    free(line_dup);
}
// commandname arg1 < inputfile > outfile
void input_output_redirection(command* cmd, char* line){
    cmd->one = malloc(sizeof(command));
    initialize(cmd->one);

    char* line_dup = strdup(line);
    char* saveptr;
    char* token = strtok_r(line_dup, "<", &saveptr);
    if(token != NULL){
        // printf("token: %s\n", token);
        parseSpaces(cmd, token);
    }
    if(saveptr != NULL){
        if(*(saveptr) == '<'){
            cmd->inputMod = 2;
            saveptr+=1;
        }else{
            cmd->inputMod = 1;
        }
    }
    // input file
    char* token1 = strtok_r(NULL, ">", &saveptr);
    if(token1 != NULL){
        // printf("input file: %s\n", token1);
        cmd->file = strdup(token1);
    }
    
    // outputfile
    if(saveptr != NULL){
        if(*(saveptr) == '>'){
            saveptr+=1;
            cmd->outputMod = 2;
        }else{
            cmd->outputMod = 1;
        }
        // printf("outfile: %s\n", saveptr);
        cmd->one->file = strdup(saveptr);
    }
    
    
}
void parseLine(command* cmd, char* line){
    // 1. initialize cmd struct
    initialize(cmd);

    // 2. parse the line
    int inputMod = 0;
    int outputMod = 0;
    for(char* ptr = line; *ptr != '\0'; ptr+=1 ){
        
        // 1. | exits
        if(*ptr == '|'){
            // puts("find |");
            parsePipe(cmd, line);
            return;
        }else if(*ptr == '&' && *(ptr+1) != '\0'){
        // 2. & in the middle
            // puts("find & in the middle");
            parseParallel(cmd, line);
            return;
        }else if(*ptr == '&' && *(ptr+1) == '\0' ){
        // 3. & at the end
            // puts("-------find | at the end");
            parseBackground(cmd, line);
            return;
        }else if(*ptr == '<'){
            // input_redirection(cmd, line);
            inputMod = 1;
            // return;
        }else if(*ptr == '>'){
            // output_redirection(cmd, line);
            outputMod = 1;
            // return;
        }else{
            // go for loop again
            ;
        }

    }
    // both < and > 
    if(inputMod == 1 && outputMod == 1){
        input_output_redirection(cmd, line);
        return;

    // only <
    }else if(inputMod == 1){
        input_redirection(cmd, line);
        return;

    // only >
    }else if(outputMod == 1){
        output_redirection(cmd, line);
        return;
    
    // 
    }else{
        parseSpaces(cmd, line);
        return;
    }
}

void my_cd(command* cmd){
    // get current dir
    static char PWD[PATH_MAX];
    getcwd(PWD, sizeof(PWD));

    // check how many arguments 
    if(cmd->argc > 2){
        puts("Only one argument is allowed.");
        return;
    }
    // argc comtains only command name
    if(cmd->argc == 1){
        puts(PWD);
        return;
    }
    if (chdir(cmd->arg[1]) == 0){
        getcwd(PWD, sizeof(PWD));
        setenv("PWD", PWD, 1);
        // printf("Successful!\n%s\n", PWD);
    }else{
        puts("failed to chdir");
    }
    
}
void my_clr(){
    printf("\033[H\033[2J");
}
void my_dir(command* cmd){
    // argc <= 2
    if(cmd->argc > 2){
        puts("Only one argument is allowed.");
        return;
    }
    // get current dir
    char PWD[PATH_MAX];
    if(getcwd(PWD, sizeof(PWD)) == NULL){
        perror("cannot get current dir");
        return;
    }
    char filePath[PATH_MAX] = "";

    // check whether argc is 1 or 2
    if(cmd->argc == 1){
        // filePath = strdup(PWD);
        strcat(filePath, PWD);
    }else{
        strcat(filePath, cmd->arg[1]);
    }
    // open dir
    DIR* dir = opendir(filePath);
    // return error if error
    if(dir == NULL){
        perror("open file failed");
        return;
    }
    struct dirent* dirInfo = NULL;
    // read item in the dir
    while((dirInfo = readdir(dir) )!= NULL){
        printf("%s\n", dirInfo->d_name);
    }
    closedir(dir);
    
}

extern char **environ;
void my_environ(){
    int i =0;
    while(environ[i] != NULL){
        printf("%s\n", environ[i]);
        i+=1;
    }
}

void my_echo(command* cmd){
    // there is only command name without arguments
    if(cmd->argc == 1){
        // print out spaces in the line
        puts("");
    }else{
        // puts("echo with argu");
        for(int i = 1; i < cmd->argc && cmd->arg[i] != NULL; i++){
            printf("%s ", cmd->arg[i]);
        }
        printf("\n");
    }
}
void my_help(){
    char PWD[PATH_MAX];
    if(getcwd(PWD, sizeof(PWD)) == NULL){
        perror("cannot get current dir");
        return;
    }
    strcat(PWD, "/readme");
    FILE* fptr = fopen(PWD, "r");

    char c;
    c = fgetc(fptr);
    while(c != EOF){
        printf("%c", c);
        c = fgetc(fptr);
    }
    fclose(fptr);
    puts("");
}

void my_pause(){
    while(getchar() != '\n'){
        ;
    }
}

void my_quit(){
    exit(EXIT_SUCCESS);
}

// int main(int argc, char const *argv[])
// {
//     command one ;
//     initialize(&one);
//     char cmd[10] = "dir";
//     parseLine(&one, cmd);
//     printCommand(&one);
//     my_dir(&one);
//     return 0;
// }

