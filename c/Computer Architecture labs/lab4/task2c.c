#include "lab4_util.h"

#define SYS_WRITE 4
#define SYS_READ 3
#define STDIN 0
#define STDERR 2
#define STDOUT 1
#define SYS_OPEN 5
#define SYS_CLOSE 6
#define SYS_GETDENT 141
#define BUF_SIZE 8192
extern int system_call();
void debugPrint(int call,int returned){
  system_call(SYS_WRITE,STDERR,itoa(call),strlen(itoa(call)));
  system_call(SYS_WRITE,STDERR,"\n",1);
  system_call(SYS_WRITE,STDERR,itoa(returned),strlen(itoa(returned)));
  system_call(SYS_WRITE,STDERR,"\n",1);
}
void debugPrintDirent(char * name,int length){
   system_call(SYS_WRITE,STDERR,name,strlen(name));
  system_call(SYS_WRITE,STDERR,"\n",1);
  system_call(SYS_WRITE,STDERR,itoa(length),strlen(itoa(length)));
  system_call(SYS_WRITE,STDERR,"\n",1);
}
struct linux_dirent {
               unsigned long  d_ino;     /* Inode number */
               unsigned long  d_off;     /* Offset to next linux_dirent */
               unsigned short d_reclen;  /* Length of this linux_dirent */
               char           d_name[];  /* Filename (null-terminated) */
                                 /* length is actually (d_reclen - 2 -
                                    offsetof(struct linux_dirent, d_name)) */
               /*
               char           pad;       // Zero padding byte
               char           d_type;    // File type (only since Linux
                                         // 2.6.4); offset is (d_reclen - 1)
               */
           };

int main (int argc , char* argv[], char* envp[])
{
  
  int fd, nread,ret;
           char buf[BUF_SIZE];
           struct linux_dirent *d;
           int bpos;
           int debug=0;
           char d_type;
           char * prefix ="";
          int i=0;
          for ( i=1;i<argc;i++){
            if(strcmp(argv[i],"-D")==0){
              debug=1;
            }
            if(strncmp(argv[i],"-p",2)==0){
                prefix = &argv[i][2];
            }
            if(strncmp(argv[i],"-a",2)==0){
                prefix = &argv[i][2];
            }
          }

  fd = system_call(SYS_OPEN,".",0,0777);
  if(debug==1){
    debugPrint(SYS_OPEN,fd);
  }
  if(fd==-1){
    system_call(1,0x55);
  }
   for ( ; ; ) {
               nread = system_call(SYS_GETDENT, fd, buf, BUF_SIZE);
               if(debug==1){
                      debugPrint(SYS_GETDENT,nread);
                 }       
                 if(nread==-1){
                     system_call(1,0x55);
                  }
                  if(nread==0){
                    break;
                  }
               for (bpos = 0; bpos < nread;) {
                   d = (struct linux_dirent *) (buf + bpos);
                   d_type = *(buf + bpos + d->d_reclen - 1);
                   if(strncmp(prefix,d->d_name,strlen(prefix))==0){
                        ret = system_call(SYS_WRITE,STDOUT,d->d_name,strlen(d->d_name));
                       /* system_call(SYS_WRITE,STDOUT,d_type,strlen(d_type));*/
                       infector(prefix);
                       system_call(SYS_WRITE,STDOUT,"\n",1) ;  
                   }
                    if(debug==1){
                      debugPrint(SYS_WRITE,ret);
                      debugPrintDirent(d->d_name,sizeof(d));
                   }   
                   
                                    
                   bpos += d->d_reclen;
               }

   }

  return 0;
}
