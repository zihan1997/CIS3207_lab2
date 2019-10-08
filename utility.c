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
    puts(line);
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
    // initialize cmd
    initialize(cmd);
    
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
        // "<"
        }else{
            cmd->inputMod = 1;
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
    // initialize cmd
    initialize(cmd);
    
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
        // ">>"
        if(*saveptr == '>'){
            cmd->outputMod = 2;
            saveptr = saveptr+1;
        // "<"
        }else{
            cmd->outputMod = 1;
        }
        // printf("saveptr: %s\n", saveptr);
        while(*saveptr == ' '){
            saveptr+=1;
        }
        cmd->file = strdup(saveptr);
    }
}
void parseLine(command* cmd, char* line){
    // 1. initialize cmd struct
    initialize(cmd);

    // 2. parse the line

    // a. gain # of |, &
    int pipeNum = 0;
    int paraNum = 0;
    for(char* ptr = line;*ptr != '\0'; ptr+=1 ){
        
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
            // puts("find | at the end");
            parseBackground(cmd, line);
            return;
        }else if(*(ptr+1) == '\0'){
            // puts("no delimitor");
            parseSpaces(cmd, line);
            return;
        }else if(*ptr == '<'){
            input_redirection(cmd, line);
            return;
        }else if(*ptr == '>'){
            output_redirection(cmd, line);
            return;
        }else{
            // go for loop again
            ;
        }

    }
}

// check if the dir is absolute or relative
int checkDir(char* path){
    if( strncmp( path, "/", 1) == 0 || strncmp(path, "./", 2) == 0 || 
                                            strncmp(path, "../", 3) == 0 ){
        return 1;
    }
    return 0;
}

void my_cd(command* cmd){
    // get current dir
    char PWD[PATH_MAX];
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
    if( checkDir(cmd->arg[1])){ // absolute path
        if (chdir(cmd->arg[1]) != 0){ 
            printf("Unsuccessful go to the dirctory:%s.\n", cmd->arg[1]);
            return;
        }
    }else{
        strcat(PWD, "/");
        strcat(PWD, cmd->arg[1]);
    
        if (chdir(PWD) != 0){ 
            printf("Unsuccessful go to the dirctory:%s.\n", PWD);
            return;
        }
    }
    // getcwd(PWD, sizeof(PWD));
    // printf("Successful!\n%s\n", PWD);
}
void my_clr(){
    printf("\033[H\033[2J");
}
void my_dir(command* cmd){
    if(cmd->argc > 2){
        puts("Only one argument is allowed.");
        return;
    }
    // get current dir
    char PWD[PATH_MAX];
    getcwd(PWD, sizeof(PWD));
    char filePath[PATH_MAX] = "";

    // no argument, act as ls
    if(cmd->argc == 1){
        // filePath = strdup(PWD);
        strcat(filePath, PWD);
    }
    // relative path
    if(!checkDir(cmd->arg[1])){
        // pwd + relative path
        strcat(filePath, PWD);
        strcat(filePath, "/");
        strcat(filePath, cmd->arg[1]);
    }
    DIR* dir = opendir(filePath);
    if(dir == NULL){
        perror("open file failed");
        return;
    }
    struct dirent* dirInfo = NULL;
    
    while((dirInfo = readdir(dir) )!= NULL){
        printf("%s\n", dirInfo->d_name);
    }
    closedir(dir);
    
}
void my_environ(command* cmd){
    ;
}

void my_echo(command* cmd){
    // there is only command name without arguments
    if(cmd->argc == 1){
        // print out spaces in the line
        puts("");
    }else{
        for(int i = 1; i <= cmd->argc && cmd->arg[i] != NULL; i++){
            printf("%s ", cmd->arg[i]);
        }
        printf("\n");
    }
}
void my_help();

void my_pause(){
    while(getchar() != '\n'){
        ;
    }
}

void my_quit(){
    ;
}

// int main(){
//     command cmd;
//     cmd.arg[0] = "echo";
//     cmd.arg[1] = ",,";
//     cmd.arg[2] = NULL;
//     cmd.argc = 2;
//     cmd.background = 0;
//     cmd.file = NULL;
//     cmd.inputMod = 0;
//     cmd.outputMod = 0;
//     cmd.parallel = 0;
//     cmd.one = NULL;
//     cmd.pipe = 0;

//     command cmd3;
//     cmd3.arg[0] = "cd";
//     cmd3.arg[1] = "xx";
//     cmd3.arg[2] = NULL;
//     cmd3.argc = 2;
//     cmd3.background = 0;
//     cmd3.file = NULL;
//     cmd3.inputMod = 0;
//     cmd3.outputMod = 0;
//     cmd3.parallel = 0;
//     cmd3.one = NULL;
//     cmd3.pipe = 0;
//     my_clr();
//     // my_cd(&cmd);
//     // my_dir(&cmd);
//     // my_echo(&cmd);
//     // my_pause();
//     // char* x = "1|2";
//     // char** line1 = parsePipe(x, &cmd);
//     // printf("%s\t%s\n", line1[0], line1[1]);
//     // parseSpaces(&cmd, "cd 1");
//     // parsePipe(&cmd1, line);
//     // parseParallel(&cmd, line);
    // command cmd1;
//     printf("\n\n\n\n\nBEGIN\n");
    // char line[100] = "cd>>x.txt";
    // parseLine(&cmd1, line);
    // parseParallel(&cmd1, line);
    // output_redirection(&cmd1, line);
    // input_redirection(&cmd1, line);
    // printCommand(&cmd1);
    // return 0;
// }