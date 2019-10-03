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
    struct commandLine* one;
};
typedef struct commandLine command;

extern char* environ[];

void mycd(command* cmd);
void my_clr();
void my_dir(command* cmd);
void my_environ(command* cmd);
void my_echo(command* cmd);
void my_help();
void my_pause();
void my_quit();



#endif