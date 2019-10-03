#include "myshell.h"
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <dirent.h>

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

int main(){
    command cmd;
    cmd.arg[0] = "dir";
    cmd.arg[1] = "";
    cmd.arg[2] = NULL;
    cmd.argc = 2;
    cmd.background = 0;
    cmd.file = NULL;
    cmd.inputMod = 0;
    cmd.outputMod = 0;
    cmd.parallel = 0;
    cmd.one = NULL;
    my_clr();
    // my_cd(&cmd);
    my_dir(&cmd);
    return 0;
}