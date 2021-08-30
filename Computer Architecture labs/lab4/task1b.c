#include "lab4_util.h"

#define SYS_WRITE 4
#define SYS_READ 3
#define STDIN 0
#define STDERR 2
#define STDOUT 1
#define SYS_OPEN 5
#define SYS_CLOSE 6
extern int system_call();
int main (int argc , char* argv[], char* envp[])
{
  int i;
  int debug =0;
  int output = STDOUT;
  int input = STDIN;
  for(i=1;i<argc;i++){
    if(strcmp(argv[i],"-D")==0){
      debug=1;
    }
    if(strncmp(argv[i],"-i",2)==0){
      input = system_call(SYS_OPEN,argv[i]+2,0,0777);
    }
    if(strncmp(argv[i],"-o",2)==0){
      output = system_call(SYS_OPEN,argv[i]+2,65,0777);
    }
  }
  char  str;
  int returnCode;
  while((returnCode=system_call(SYS_READ,input, &str,1))!=0){
      if(debug==1){
        
        char * tmp;
        tmp = itoa(returnCode);
        system_call(SYS_WRITE,STDERR,tmp, strlen(tmp));
        system_call(SYS_WRITE,STDERR,"\n", 2);
        tmp = itoa(input);
        system_call(SYS_WRITE,STDERR,tmp, strlen(tmp));
        system_call(SYS_WRITE,STDERR,"\n", 2);
      }
      if(str>='A'&& str<= 'Z'){
        str+='a'-'A';
      }
      returnCode =system_call(SYS_WRITE,output,&str, 1);
       if(debug==1){
        char * debugPrint = "";
        char * tmp; 
        tmp = itoa(returnCode);
        system_call(SYS_WRITE,STDERR,tmp, strlen(tmp));
        system_call(SYS_WRITE,STDERR,"\n", 2);
        tmp = itoa(output);
        system_call(SYS_WRITE,STDERR,tmp, strlen(tmp));
        system_call(SYS_WRITE,STDERR,"\n", 2);
      }
  }
	system_call(SYS_CLOSE,input);
	system_call(SYS_CLOSE,output);

  return 0;
}
