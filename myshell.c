#include "myshell.h"
#include "utility.c"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

int main(int argc, char const *argv[], char* envp[])
{
    // 1. get current dir
    char PWD[PATH_MAX];
    if(getcwd(PWD, sizeof(PWD)) == NULL){
        perror("cannot get current dir");
        return EXIT_FAILURE;
    }
    // 2. set shell environment
    setenv("shell", PWD, 1);
    // 3. create new command
    command cmd;
    // 4. initialize struct
    initialize(&cmd);
    // 5. get line
    char* line = NULL;
    size_t size;

    // batch mode
    FILE* batch = stdin;
    if(argc == 2){
        batch = fopen(argv[1], "r");
        if( batch == NULL){
            perror("open file failed");
            return EXIT_FAILURE;
        }
    }
    int flag = 1;
    while(flag == 1){
        
        if(batch == stdin){
            printf("myshell:%s> ", get_current_dirname(PWD));
        }        
        int len = getline(&line, &size, batch);
        if(len > 0){
            // remove \n or \r at the end
            while (len > 0 && (line[len - 1] == '\n' || line[len-1] == '\r')) {
                line[len-1] = '\0';
                len -= 1;
            }
            parseLine(&cmd, line);
            if( strcmp(cmd.arg[0], "quit") == 0 ){
                // printf("Break;\n");
                flag = 0;
                break;
            } 
            // for debug
            // printCommand(&cmd);
            run_shell(&cmd);
        }else{
            if(len < 0){
                printf("line < 0\n");
            }
            // line is empty
            continue;
        }
    }

    freeStruct(&cmd);
    return 0;
}
char* get_current_dirname(){
    char* pwd = getcwd(NULL, 0);
    char* saveptr;
    char* token = strtok_r(pwd, "/", &saveptr);
    while(saveptr != NULL){
        token = strtok_r(NULL, "/", &saveptr);
    }
    return token;
    
}
// single internal commands
int runInternalCmd(command* cmd){
    // copy the command name to cmdName
    char* cmdName = cmd->arg[0];
    // printf("cmd name: %s\n", cmd->arg[0]);
    // compare if matches
    if(strcmp(cmdName, "cd") == 0){
        my_cd(cmd);
        return 1;
    }else if(strcmp(cmdName, "clr") == 0){
        my_clr();
        return 1;
    }else if(strcmp(cmdName, "dir") == 0){
        puts("dir");
        my_dir(cmd);
        return 1;
    }else if(strcmp(cmdName, "environ") == 0){
        my_environ();
        return 1;
    }else if(strcmp(cmdName, "echo") == 0){
        my_echo(cmd);
        return 1;
    }else if(strcmp(cmdName, "Help") == 0){
        my_help();
        return 1;
    }else if(strcmp(cmdName, "pause") == 0){
        my_pause();
        return 1;
    // }else if(strcmp(cmdName, "quit") == 0){
    //     // free(cmd);
    //     my_quit();
    //     return 1;
    }else{
        // not internal commands
        return 0;
    }
}
void build_fork(command* cmd){
    pid_t childPID;
    childPID = fork();
    // fork successful
    if(childPID >= 0){
        //child process
        if(childPID == 0){
            execvp(cmd->arg[0], cmd->arg);
        }else{
            waitpid(childPID, 0, 0);
        }
    }else{
        puts("Fork failed");
    }
}


// no pip involved
// no background involved
void executeSingleCommand(command* cmd){
    // create fork for executing input|output redirection | background
    pid_t childPID = fork();
    if(childPID < 0){
        puts("Fork failed");
        return;
    }else{ // fork succeed
        // parent process
        if(childPID != 0){
            waitpid(childPID, 0, 0);
            if(runInternalCmd(cmd) == 1){
                return;
            }
            return;
        }else{
            // do execvp
            // input/output redirections
            int fd = 0;
            int oflag;
            // // 1. both input and output
            // if(cmd->inputMod != 0 && cmd->)
            // 2. input redirection
            if(cmd->inputMod != 0){
                oflag = O_RDONLY;
                fd = open(cmd->file, oflag);
                // error happens
                if(fd == -1){
                    printf("Cannot open file\n");
                    return;
                }else{
                    dup2(fd, STDIN_FILENO);
                    // execvp(cmd->arg[0], cmd->arg);
                }
            }
            if(cmd->outputMod != 0){
            // 3. output redirection
                oflag |= O_CREAT|O_WRONLY;
                // command has >>
                if(cmd->outputMod == 2){
                    // append
                    oflag |= O_APPEND;
                }else{
                    oflag |= O_TRUNC;
                }
                // check have both input and output redirection
                if(cmd->inputMod != 0){
                    fd = open(cmd->one->file, oflag, S_IRWXU|S_IRWXG|S_IRWXO);
                }else{
                    fd = open(cmd->file, oflag, S_IRWXU|S_IRWXG|S_IRWXO);
                }
                if(fd == -1){
                    printf("Cannot open file\n");
                    return;
                }else{
                    dup2(fd, STDOUT_FILENO);
                    // execvp(cmd->arg[0], cmd->arg);
                }
            }
            exit(execvp(cmd->arg[0], cmd->arg));
        }

    }
    
}
void run_pipe(command* cmd){
    if(cmd->one == NULL){
        perror("only one command. Cannot pipe");
        return;
    }
    int pfds[2];
    // create pipe
    if(pipe(pfds) == 0){
        // child
        if(fork() == 0){
            // close stdout
            close(1);
            // write into pipe
            dup2(pfds[1], STDOUT_FILENO);
            close(pfds[0]);
            // execlp("ls", "ls", "-1", NULL);
            execvp(cmd->arg[0], cmd->arg);
            exit(EXIT_SUCCESS);
            // executeSingleCommand(cmd);
        }else{
            // parent

            // close stdin
            close(0);
            // write stdin into pipe
            dup2(pfds[0], STDIN_FILENO);
            close(pfds[1]);
            // execlp("wc", "wc", "-l", NULL);
            execvp(cmd->one->arg[0], cmd->one->arg);
            return;
            // executeSingleCommand(cmd->one);
        }
    }
}
void run_shell(command* cmd){
    if(cmd->background || cmd->parallel){
        while(cmd){
            build_fork(cmd);
            cmd = cmd->one;
        }
    }
    if(cmd->pipe){
        run_pipe(cmd);
    }else{
        executeSingleCommand(cmd);
    }
}
/*
int main(int argc, char const *argv[], char* envp[])
{
    environ = envp;
    char PWD[PATH_MAX];
    if(getcwd(PWD, sizeof(PWD)) == NULL){
        perror("cannot get current dir");
        return 1;
    }
    // set shell environment
    setenv("shell", PWD, 1);
    
    command cmd;
    initialize(&cmd);
    // char line[100] = "ls -l|wc -l";
    // char line[100] = "ps|grep root";
    // char line[100] = "wc<pseudocode.txt>>a.txt";
    char line[100] = "cd 1";
    parseLine(&cmd, line);
    printCommand(&cmd);
    // executeSingleCommand(&cmd);
    // run_shell(&cmd);
    my_cd(&cmd);
    return 0;
}
*/