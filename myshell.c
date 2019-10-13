#include "myshell.h"
#include "utility.c"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

// int main(int argc, char const *argv[])
// {
//     // 1. create new command
//     command cmd;
//     // 2. initialize struct
//     initialize(&cmd);
//     //3. check b
//     char* line = NULL;
//     size_t size;

//     while(1){
//         printf("myshell> ");
//         int len = getline(&line, &size, stdin);
//         // remove \n or \r at the end
//         while (len > 0 && (line[len - 1] == '\n' || line[len-1] == '\r')) {
//             line[len-1] = '\0';
//             len -= 1;
//         }
//         parseLine(&cmd, line);
//         // printCommand(&cmd);
//     }
//     return 0;
// }

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
    }else if(strcmp(cmdName, "quit") == 0){
        my_quit();
        return 1;
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
            return;
        }else{
            // do execvp
            // input/output redirections
            int fd;
            int oflag;
            // 1. input redirection
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
            // 2. output redirection
                oflag = O_CREAT|O_WRONLY;
                // command has >>
                if(cmd->outputMod == 2){
                    // append
                    oflag |= O_APPEND;
                }else{
                    oflag |= O_TRUNC;
                }
                fd = open(cmd->file, oflag, S_IRWXU);
                if(fd == -1){
                    printf("Cannot open file\n");
                    return;
                }else{
                    dup2(fd, STDOUT_FILENO);
                    // execvp(cmd->arg[0], cmd->arg);
                }
            }
            // if(cmd->background != 0){
            //     build_fork(cmd);
            // }
            if(runInternalCmd(cmd) == 1){
                return;
            }
            execvp(cmd->arg[0], cmd->arg);
        }

    }
    
}
void run_shell_pip(command* cmd){
    if(cmd->background != 0){
        pid_t childPID = fork();
        if(childPID < 0){
            printf("fork failed\n");
            return;
        }
        // parent
        if(childPID != 0){
            return;
        }else{ // child
            executeSingleCommand(cmd);
        }
    }else if(cmd->pipe != 0){
        // make sure there are two commands
        if(cmd->one != NULL){
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
                    // execvp(cmd->arg[0], cmd->arg);
                    executeSingleCommand(cmd);
                }else{
                    // parent

                    // close stdin
                    close(0);
                    // write stdin into pipe
                    dup2(pfds[0], STDIN_FILENO);
                    close(pfds[1]);
                    // execlp("wc", "wc", "-l", NULL);
                    // execvp(cmd->one->arg[0], cmd->one->arg);
                    executeSingleCommand(cmd->one);
                }
            }
        }
    }
}

int main(int argc, char const *argv[], char* envp[])
{
    environ = envp;
    char PWD[PATH_MAX];
    if(getcwd(PWD, sizeof(PWD)) == NULL){
        perror("cannot get current dir");
        return 1;
    }
    
    command cmd;
    initialize(&cmd);
    // char line[100] = "ls -l|wc -l ";
    // char line[100] = "ps|grep root&";
    char line[100] = "dir";
    parseLine(&cmd, line);
    printCommand(&cmd);
    // executeSingleCommand(&cmd);
    // run_shell_pip(&cmd);
    return 0;
}
