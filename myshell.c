#include "myshell.h"
#include "utility.c"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char const *argv[])
{
    // 1. create new command
    command cmd;
    // 2. initialize struct
    initialize(&cmd);
    //3. check b
    char* line = NULL;
    size_t size;

    // while(1){
        printf("myshell> ");
        int len = getline(&line, &size, stdin);
        // remove \n at the end
        while (len > 0 && (line[len - 1] == '\n' || line[len-1] == '\r')) {
            line[len-1] = '\0';
            len -= 1;
        }
        parseLine(&cmd, line);
        printCommand(&cmd);
    // }
    return 0;
}
