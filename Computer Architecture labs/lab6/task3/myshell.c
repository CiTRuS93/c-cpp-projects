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

       #include <sys/stat.h>
       #include <fcntl.h>

#define TERMINATED -1
#define RUNNING 1
#define SUSPENDED 0
#define maxSize 2048
extern int errno;
typedef struct process{
    cmdLine* cmd;                         /* the parsed command line*/
    pid_t pid; 		                  /* the process id that is running the command*/
    int status;                           /* status of the process: RUNNING/SUSPENDED/TERMINATED */
    struct process *next;	                  /* next process in chain */
} process;
process *mylist=NULL;
void freeProcessList(process* process_list){
    process *p = process_list;
  while (p != NULL)
  {
    process* tmp=p;
    p = p->next;
    
    freeCmdLines(tmp->cmd);
    
    free(tmp);
    
  }
  
}
void updateProcessStatus(process* process_list, int pid, int status){
  process* p = process_list;
  while(p!=NULL){
    if(p->pid==pid){
      p->status=status;
      return;
    }
    p=p->next;
  }
}
void updateProcessList(process **process_list){
  process *p = *process_list;
  while (p != NULL)
  {
    int i = waitpid(p->pid, &p->status, WNOHANG);
    if (i == -1)
      {
        p->status = TERMINATED;
      }
    else if (WIFSTOPPED(p->status))
      {
        p->status = SUSPENDED;
      }
    else if (WIFCONTINUED(p->status))
      {
        p->status = RUNNING;
      }
    p = p->next;
  }

}
void addProcess(process** process_list, cmdLine* cmd,pid_t pid){
  process *p = (process*) malloc(sizeof(process));
  p->cmd = cmd;
  p->pid = pid;
  p->status=RUNNING;
  p->next = *process_list;
  *process_list = p;
  
}
void printProcessList(process** process_list){
  updateProcessList(process_list);
  process *p = *process_list;
  process *prev=NULL;
  printf("PID \t Command\t STATUS\n");
  while(p!=NULL){
    printf("%d \t %s \t\t",p->pid,p->cmd->arguments[0]);
    if(p->status==RUNNING){
      printf("RUNNING\n");
    }
    if(p->status==SUSPENDED){
      printf("SUSPENDED\n");
    }
    if(p->status==TERMINATED){
      printf("TERMINATED\n");
    }
    if(prev==NULL){
      if(p->status==TERMINATED){
        process *tmp =p;
        p=p->next;
        *process_list = p;
        freeCmdLines(tmp->cmd);
        free(tmp);
      }
      else{
        prev=p;
        p=p->next;
      }
    }
    else if(p->status==TERMINATED){
        process *tmp =p;
        p=p->next;
        prev->next=p;
        freeCmdLines(tmp->cmd);
        free(tmp);
      }
      else{
        prev=p;
        p=p->next;
      }
    
    
  }

}
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
      _exit(errno);
    }
    addProcess(&mylist,pCmdLine,id);
    if (id == 0)
    {
      int input=-1;
      int output=-1;
      
            if(pCmdLine->inputRedirect!=NULL){
              input=open(pCmdLine->inputRedirect,O_RDONLY);
              close(STDIN_FILENO);
              int fd = dup(input);
              if(fd==-1){
                perror("The following error occurred");
                printf("Value of errno: %d\n", errno);
                 freeCmdLines(pCmdLine);
                 _exit(errno);
              }
            }
            if(pCmdLine->outputRedirect!=NULL){
              output=open(pCmdLine->outputRedirect,O_CREAT|O_WRONLY|S_IRUSR|S_IWUSR|S_IRWXU );
              close(STDOUT_FILENO);

              int of = dup(output);
              if(of==-1){
                 perror("The following error occurred");
                printf("Value of errno: %d\n", errno);
                 freeCmdLines(pCmdLine);
                 _exit(errno);
              }
              
            }
            
            i = execvp(pCmdLine->arguments[0], pCmdLine->arguments);
      if(input!=-1){
        close(input);
      }
      if(output!=-1){{
        close(output);
      }}
    }
   

    if (id != 0 && pCmdLine->blocking == 1)
    {
      int *wstatus=0;
      if (waitpid(id, wstatus, 0) == -1)
      {
        perror("The following error occurred");
        printf("Value of errno: %d\n", errno);
        freeCmdLines(pCmdLine);
        _exit(errno);
      }
    }
     
    return i;
  }

int executePipe(cmdLine * pCmdLine){
  int pipefd[2];
  pipe(pipefd);
  int i;
  int id1= fork();
  if(id1==0){
    close(STDOUT_FILENO);
    dup(pipefd[1]);
    close(pipefd[1]);
    i = execvp(pCmdLine->arguments[0], pCmdLine->arguments);
  }
  close(pipefd[1]);
  cmdLine * next = pCmdLine->next;
  int id2 = fork();
  if(id2==0){
    close(STDIN_FILENO);
    dup(pipefd[0]);
    close(pipefd[0]);
    i = execvp(next->arguments[0], next->arguments);
  }
  close(pipefd[0]);
       waitpid(id1,NULL,0); 
      waitpid(id2,NULL,0); 
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
        freeProcessList(mylist);
        exit(0);
      }
      else if (strcmp(pCmdLine->arguments[0], "cd") == 0)
      {
        
        if (chdir(pCmdLine->arguments[1]) == -1)
        {
            perror("The following error occurred");
            printf("Value of errno: %d\n", errno);
            freeCmdLines(pCmdLine);
            _exit(errno);
        }
        freeCmdLines(pCmdLine);
        
      }
      else if (strcmp(pCmdLine->arguments[0], "procs") == 0)
      {
          
        printProcessList(&mylist);
        freeCmdLines(pCmdLine);
      }
      else if (strcmp(pCmdLine->arguments[0], "suspend") == 0)
      {
        int i = atoi(pCmdLine->arguments[1]);
        kill(i,SIGTSTP);
        updateProcessStatus(mylist,i,SUSPENDED);
        freeCmdLines(pCmdLine);
        
      }else if  (strcmp(pCmdLine->arguments[0], "kill") == 0)
      {
          
        int i = atoi(pCmdLine->arguments[1]);
        kill(i,SIGINT);
        updateProcessStatus(mylist,i,TERMINATED);
        freeCmdLines(pCmdLine);
      }
      else if (strcmp(pCmdLine->arguments[0], "wake") == 0)
      {
        int i = atoi(pCmdLine->arguments[1]);
        kill(i,SIGCONT);
        updateProcessStatus(mylist,i,RUNNING);
        freeCmdLines(pCmdLine);
      }
      else{ 
        if(pCmdLine->next==NULL){
          i = execute(pCmdLine);
        }else{
          executePipe(pCmdLine);
        }
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
