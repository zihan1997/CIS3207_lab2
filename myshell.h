#ifndef MYSHELL_H_
#define MYSHELL_H_

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



#endif