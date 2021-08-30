#include <stdlib.h>
#include <stdio.h>
#include "LineParser.h"
#include <string.h>
#include <unistd.h>
#include <limits.h>
//all the error handling from stackOverFlow
#include <errno.h>
#include <signal.h>
#include <wait.h>
#include <sys/types.h>
#define TERMINATED -1
#define RUNNING 1
#define SUSPENDED 0
#define maxSize 2048
int debug = 0;
int execute(cmdLine * pCmdLine)
  {
      int i = 0;
    pid_t id = 0;
    id = fork();
    if(debug==1){
        fprintf(stderr,"pid: %d\n",id);
    }
    if (id == -1)
    {
      perror("The following error occurred");
      printf("Value of errno: %d\n", errno);
      freeCmdLines(pCmdLine);
      exit(errno);
    }
    if (id == 0)
    {

      
      i = execvp(pCmdLine->arguments[0], pCmdLine->arguments);

    }
   

    if (id != 0 && pCmdLine->blocking == 1)
    {
      //int *wstatus;
      if (waitpid(id, 0, 0) == -1)
      {
        perror("The following error occurred");
        printf("Value of errno: %d\n", errno);
        freeCmdLines(pCmdLine);
        exit(errno);
      }
    }
     freeCmdLines(pCmdLine);
    return i;
  }
int main(int argc, char *argv[])
  {
    
    cmdLine *pCmdLine;
    for (int i = 1; i < argc; i++)
      if (strcmp(argv[i], "-d") == 0) {
        debug = 1;
      }
    char buf[maxSize];
    char buffer1[PATH_MAX];
    int i = 0;
    
    do
    {
        getcwd(buffer1, PATH_MAX);
        printf("%s>>",buffer1);
      fgets(buf, maxSize, stdin);
      pCmdLine = parseCmdLines(buf);
        if(debug==1){
            for(int i=0;i<pCmdLine->argCount;i++){
                fprintf(stderr,"%s ",pCmdLine->arguments[i]);
            }
            fprintf(stderr,"\n");
        }
      if (strcmp(pCmdLine->arguments[0], "quit") == 0)
      {
        freeCmdLines(pCmdLine);
        exit(0);
      }
      else if (strcmp(pCmdLine->arguments[0], "cd") == 0)
      {
          
        if (chdir(pCmdLine->arguments[1]) == -1)
        {
            perror("The following error occurred");
            printf("Value of errno: %d\n", errno);
            freeCmdLines(pCmdLine);
            exit(errno);
        }
        
        freeCmdLines(pCmdLine);
      }
      else{ i = execute(pCmdLine);
      if (i == -1)
      {
        perror("The following error occurred");
        printf("Value of errno: %d\n", errno);
        freeCmdLines(pCmdLine);
        _exit(errno);
      }
      }
    } while (1);
  }
