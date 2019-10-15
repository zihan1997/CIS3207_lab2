#ifndef MYSHELL_H_
#define MYSHELL_H_
// #include "utility.c"

struct commandLine
{
    char* arg[100]; // all arguments
    int argc; // the number of arguments
    char* file; 
    int inputMod;  // < or << read in
    int outputMod; // > or >> write out
    int background; // & at the end?
    int parallel;
    int pipe;
    struct commandLine* one;
};
typedef struct commandLine command;

// myshell.c
int runInternalCmd(command* cmd);
void build_fork(command* cmd);
void executeSingleCommand(command* cmd);
void run_shell(command* cmd);

// utility.c
void initialize(command* cmd);
void freeStruct(command* cmd);
void printCommand(command* cmd);
void parsePipe(command* cmd, char* line);
void parseSpaces(command* cmd, char* line);
void parseBackground(command* cmd, char* line);
void parseParallel(command* cmd, char* line);
void input_redirection(command* cmd, char* line);
void output_redirection(command* cmd, char* line);
void parseLine(command* cmd, char* line);
void mycd(command* cmd);
void my_clr();
void my_dir(command* cmd);
void my_environ();
void my_echo(command* cmd);
void my_help();
void my_pause();
extern void my_quit();



#endif